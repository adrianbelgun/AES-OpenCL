#include "stdafx.h"

// Parses the command line line arguments returning the value of the specified argument
// Argument format is --arg=value
int GetArgInt(const int argc, char** argv, const char* sArg)
{
	// Pre-calculate the size of sArgs, will be using it alot.
	size_t iStrLen = strlen (sArg);

	// Find argument 
	for (int i = 0; i < argc; i++)
	{
		if (strlen(argv[i]) > (iStrLen + 3) &&
			(*(argv[i]+0) == '-') && 
			(*(argv[i]+1) == '-') && 
			(*(argv[i] + iStrLen + 2) == '=') &&
			(strncmp(sArg, argv[i]+2, iStrLen) == 0)
			)
		{
			return atoi(argv[i] + iStrLen + 3);
		}
	}

	return 0;
}

char* GetArgString(const int argc, char** argv, const char* sArg)
{
	// Pre-calculate the size of sArgs, will be using it alot.
	size_t iStrLen = strlen (sArg);

	// Find argument 
	for (int i = 0; i < argc; i++)
	{
		if (strlen(argv[i]) > (iStrLen + 3) &&
			(*(argv[i]+0) == '-') && 
			(*(argv[i]+1) == '-') && 
			(*(argv[i] + iStrLen + 2) == '=') &&
			(strncmp(sArg, argv[i]+2, iStrLen) == 0)
			)
		{
			return (argv[i] + iStrLen + 3);
		}
	}

	return NULL;
}

#pragma region SBOX
unsigned char AES_SBox[256] = 
{
   0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
   0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
   0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
   0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
   0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
   0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
   0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
   0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
   0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
   0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
   0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
   0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
   0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
   0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
   0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
   0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};
#pragma endregion
#pragma region Rcon
unsigned char Rcon[256] = 
{
	0x8D, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36, 0x6C, 0xD8, 0xAB, 0x4D, 0x9A, 
	0x2F, 0x5E, 0xBC, 0x63, 0xC6, 0x97, 0x35, 0x6A, 0xD4, 0xB3, 0x7D, 0xFA, 0xEF, 0xC5, 0x91, 0x39, 
	0x72, 0xE4, 0xD3, 0xBD, 0x61, 0xC2, 0x9F, 0x25, 0x4A, 0x94, 0x33, 0x66, 0xCC, 0x83, 0x1D, 0x3A, 
	0x74, 0xE8, 0xCB, 0x8D, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36, 0x6C, 0xD8, 
	0xAB, 0x4D, 0x9A, 0x2F, 0x5E, 0xBC, 0x63, 0xC6, 0x97, 0x35, 0x6A, 0xD4, 0xB3, 0x7D, 0xFA, 0xEF, 
	0xC5, 0x91, 0x39, 0x72, 0xE4, 0xD3, 0xBD, 0x61, 0xC2, 0x9F, 0x25, 0x4A, 0x94, 0x33, 0x66, 0xCC, 
	0x83, 0x1D, 0x3A, 0x74, 0xE8, 0xCB, 0x8D, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 
	0x36, 0x6C, 0xD8, 0xAB, 0x4D, 0x9A, 0x2F, 0x5E, 0xBC, 0x63, 0xC6, 0x97, 0x35, 0x6A, 0xD4, 0xB3, 
	0x7D, 0xFA, 0xEF, 0xC5, 0x91, 0x39, 0x72, 0xE4, 0xD3, 0xBD, 0x61, 0xC2, 0x9F, 0x25, 0x4A, 0x94, 
	0x33, 0x66, 0xCC, 0x83, 0x1D, 0x3A, 0x74, 0xE8, 0xCB, 0x8D, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 
	0x40, 0x80, 0x1B, 0x36, 0x6C, 0xD8, 0xAB, 0x4D, 0x9A, 0x2F, 0x5E, 0xBC, 0x63, 0xC6, 0x97, 0x35, 
	0x6A, 0xD4, 0xB3, 0x7D, 0xFA, 0xEF, 0xC5, 0x91, 0x39, 0x72, 0xE4, 0xD3, 0xBD, 0x61, 0xC2, 0x9F, 
	0x25, 0x4A, 0x94, 0x33, 0x66, 0xCC, 0x83, 0x1D, 0x3A, 0x74, 0xE8, 0xCB, 0x8D, 0x01, 0x02, 0x04, 
	0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36, 0x6C, 0xD8, 0xAB, 0x4D, 0x9A, 0x2F, 0x5E, 0xBC, 0x63, 
	0xC6, 0x97, 0x35, 0x6A, 0xD4, 0xB3, 0x7D, 0xFA, 0xEF, 0xC5, 0x91, 0x39, 0x72, 0xE4, 0xD3, 0xBD, 
	0x61, 0xC2, 0x9F, 0x25, 0x4A, 0x94, 0x33, 0x66, 0xCC, 0x83, 0x1D, 0x3A, 0x74, 0xE8, 0xCB, 0x8D
};
#pragma endregion

void ComputeRoundKeys(unsigned char** roundKeys, int* rounds, size_t size,
	unsigned char* key)
{
	// Size in bytes
	switch (size)
	{
	case 16: *rounds = 11; break;
	case 24: *rounds = 12; break;
	case 32: *rounds = 15; break;
	default:
		printf("Key be 16, 24 or 32 bytes\n");
		exit(1);
	}
	
	*roundKeys = new unsigned char[*rounds * 16];
	
	unsigned char rotWord[4];

	//	The first n bytes of the expanded key are simply the encryption key.
	for (int k = 0; k < size; k++)
	{
		(*roundKeys)[k] = key[k];
	}
	for (int k = 1; k < (*rounds) ; k++)
	{
		size_t offset = size + (k - 1) * 16; // in bytes

		// Calculate the rotated word
		rotWord[0] = AES_SBox[(*roundKeys)[offset - 3]];
		rotWord[1] = AES_SBox[(*roundKeys)[offset - 2]];
		rotWord[2] = AES_SBox[(*roundKeys)[offset - 1]];
		rotWord[3] = AES_SBox[(*roundKeys)[offset - 4]];
	
		// First word
		(*roundKeys)[offset +  0] = (*roundKeys)[offset - 16] ^ rotWord[0] ^ 
			Rcon[k];
		(*roundKeys)[offset +  1] = (*roundKeys)[offset - 15] ^ rotWord[1];
		(*roundKeys)[offset +  2] = (*roundKeys)[offset - 14] ^ rotWord[2];
		(*roundKeys)[offset +  3] = (*roundKeys)[offset - 13] ^ rotWord[3];

		// Second, third and forth words
		((unsigned int *)(*roundKeys))[offset/4 + 1] = 
			((unsigned int *)(*roundKeys))[offset/4 + 0] ^ 
			((unsigned int *)(*roundKeys))[offset/4 - 3];
		((unsigned int *)(*roundKeys))[offset/4 + 2] = 
			((unsigned int *)(*roundKeys))[offset/4 + 1] ^ 
			((unsigned int *)(*roundKeys))[offset/4 - 2];
		((unsigned int *)(*roundKeys))[offset/4 + 3] = 
			((unsigned int *)(*roundKeys))[offset/4 + 2] ^ 
			((unsigned int *)(*roundKeys))[offset/4 - 1];
	}
}

ImageData ReadImageFile (char* name)
{
	Magick::Image image(name);

	size_t columns = image.columns();
	size_t rows    = image.rows();

	size_t imageSize = columns * rows * 4 * sizeof(MagickCore::Quantum);
	size_t paddedImageSize = ((imageSize + 15)/16)*16;
	if (imageSize == paddedImageSize)
		paddedImageSize += 16;
	printf("imageSize = %d MB\n", imageSize >> 20);
	
	MagickCore::Quantum* data = (MagickCore::Quantum*) malloc(paddedImageSize);

	memcpy(data, image.getPixels(0,0,columns,rows), imageSize);

	for (int k = imageSize; k < paddedImageSize; k++)
		((unsigned char*) data)[k] = (unsigned char) 
		(paddedImageSize - imageSize);
	
	ImageData img;
	img.columns = columns;
	img.rows = rows;
	img.bytes = imageSize;
	img.padded_bytes = paddedImageSize;
	img.data = (unsigned char*) data;
	
	return img;
}

void WriteImageFile (char* name, ImageData img)
{
	Magick::Image image(Magick::Geometry(img.columns, img.rows), "white");
	image.transparent("white");
	memcpy(image.getPixels(0, 0, img.columns, img.rows), img.data, 
		img.columns * img.rows * 4 * sizeof(MagickCore::Quantum));
	image.write(name);
}

void PrintPlatformInfo(cl_platform_id platformId)
{
	cl_int err = 0;
	// Get Required Size
	size_t length;
	err = clGetPlatformInfo(platformId, CL_PLATFORM_NAME, NULL, NULL, &length);
	CHECK_OPENCL_ERROR(err);
	char* sInfo = new char[length];
 	err = clGetPlatformInfo(platformId, CL_PLATFORM_NAME, length, sInfo, NULL);
	CHECK_OPENCL_ERROR(err);
	printf("%s\n", sInfo);
 	delete[] sInfo;
}

void PrintDeviceInfo(cl_device_id deviceId)
{
	cl_int err = 0;

	// Get Required Size
	size_t length;
	err = clGetDeviceInfo(deviceId, CL_DEVICE_NAME, NULL, NULL, &length);
	// Get actual device name
	char* sInfo = new char[length];
 	err = clGetDeviceInfo(deviceId, CL_DEVICE_NAME, length, sInfo, NULL);
	printf("%s\n", sInfo);
	 
	cl_ulong size;
	err = clGetDeviceInfo(deviceId, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(size),
		&size, NULL);
	CHECK_OPENCL_ERROR(err);
	printf("Total device memory: %d MB\n", size >> 20);
	
	err = clGetDeviceInfo(deviceId, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(size),
		&size, NULL);
	CHECK_OPENCL_ERROR(err);
	printf("Maximum buffer size: %d MB\n", size >> 20);
	
 	delete[] sInfo;

}

std::string GetFileContents(const char *filename)
{
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  if (in)
  {
    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    return(contents);
  }
  return "";
}

bool FileExists(const char * filename)
{
    if (FILE * file = fopen(filename, "r"))
    {
        fclose(file);
        return true;
    }
    return false;
}

double GetTime()
{
	LARGE_INTEGER frequency;				// Ticks per Second
    LARGE_INTEGER ticks;					// Ticks
	
    QueryPerformanceFrequency(&frequency);	// Get Ticks per Second
	QueryPerformanceCounter(&ticks);		// Get Start Time

	return double(ticks.QuadPart) / frequency.QuadPart;
}


void CHECK_OPENCL_ERROR(cl_int err)
{
	if (err != CL_SUCCESS)
	{
		switch (err)
		{
		case CL_DEVICE_NOT_FOUND: 
			printf("CL_DEVICE_NOT_FOUND\n"); exit(-1); 
		case CL_DEVICE_NOT_AVAILABLE:	
			printf("CL_DEVICE_NOT_AVAILABLE\n"); exit(-1); 
		case CL_COMPILER_NOT_AVAILABLE:	
			printf("CL_COMPILER_NOT_AVAILABLE\n"); exit(-1); 
		case CL_MEM_OBJECT_ALLOCATION_FAILURE:
			printf("CL_MEM_OBJECT_ALLOCATION_FAILURE\n"); exit(-1); 
		case CL_OUT_OF_RESOURCES:				
			printf("CL_OUT_OF_RESOURCES\n"); exit(-1); 
		case CL_OUT_OF_HOST_MEMORY:				
			printf("CL_OUT_OF_HOST_MEMORY\n"); exit(-1); 
		case CL_PROFILING_INFO_NOT_AVAILABLE:	
			printf("CL_PROFILING_INFO_NOT_AVAILABLE\n"); exit(-1); 
		case CL_MEM_COPY_OVERLAP:				
			printf("CL_MEM_COPY_OVERLAP\n"); exit(-1); 
		case CL_IMAGE_FORMAT_MISMATCH:			
			printf("CL_IMAGE_FORMAT_MISMATCH\n"); exit(-1); 
		case CL_IMAGE_FORMAT_NOT_SUPPORTED:		
			printf("CL_IMAGE_FORMAT_NOT_SUPPORTED\n"); exit(-1); 
		case CL_BUILD_PROGRAM_FAILURE:			
			printf("CL_BUILD_PROGRAM_FAILURE\n"); exit(-1); 
		case CL_MAP_FAILURE:					
			printf("CL_MAP_FAILURE\n"); exit(-1); 
		case CL_MISALIGNED_SUB_BUFFER_OFFSET:	
			printf("CL_MISALIGNED_SUB_BUFFER_OFFSET\n"); exit(-1); 
		case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
			printf("CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST\n"); exit(-1);
		case CL_COMPILE_PROGRAM_FAILURE: 
			printf("CL_COMPILE_PROGRAM_FAILURE\n"); exit(-1); 
		case CL_LINKER_NOT_AVAILABLE: 
			printf("CL_LINKER_NOT_AVAILABLE\n"); exit(-1); 
		case CL_LINK_PROGRAM_FAILURE: 
			printf("CL_LINK_PROGRAM_FAILURE\n"); exit(-1); 
		case CL_DEVICE_PARTITION_FAILED: 
			printf("CL_DEVICE_PARTITION_FAILED\n"); exit(-1); 
		case CL_KERNEL_ARG_INFO_NOT_AVAILABLE: 
			printf("CL_KERNEL_ARG_INFO_NOT_AVAILABLE\n"); exit(-1); 

		case CL_INVALID_VALUE: 
			printf("CL_INVALID_VALUE\n"); exit(-1); 
		case CL_INVALID_DEVICE_TYPE: 
			printf("CL_INVALID_DEVICE_TYPE\n"); exit(-1); 
		case CL_INVALID_PLATFORM:
			printf("CL_INVALID_PLATFORM\n"); exit(-1); 
		case CL_INVALID_DEVICE:
			printf("CL_INVALID_DEVICE\n"); exit(-1); 
		case CL_INVALID_CONTEXT:
			printf("CL_INVALID_CONTEXT\n"); exit(-1); 
		case CL_INVALID_QUEUE_PROPERTIES:
			printf("CL_INVALID_QUEUE_PROPERTIES\n"); exit(-1); 
		case CL_INVALID_COMMAND_QUEUE: 
			printf("CL_INVALID_COMMAND_QUEUE\n"); exit(-1); 
		case CL_INVALID_HOST_PTR: 
			printf("CL_INVALID_HOST_PTR\n"); exit(-1); 
		case CL_INVALID_MEM_OBJECT: 
			printf("CL_INVALID_MEM_OBJECT\n"); exit(-1); 
		case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: 
			printf("CL_INVALID_IMAGE_FORMAT_DESCRIPTOR\n"); exit(-1); 
		case CL_INVALID_IMAGE_SIZE:
			printf("CL_INVALID_IMAGE_SIZE\n"); exit(-1); 
		case CL_INVALID_SAMPLER:
			printf("CL_INVALID_SAMPLER\n"); exit(-1); 
		case CL_INVALID_BINARY:
			printf("CL_INVALID_BINARY\n"); exit(-1); 
		case CL_INVALID_BUILD_OPTIONS:
			printf("CL_INVALID_BUILD_OPTIONS\n"); exit(-1); 
		case CL_INVALID_PROGRAM: 
			printf("CL_INVALID_PROGRAM\n"); exit(-1); 
		case CL_INVALID_PROGRAM_EXECUTABLE: 
			printf("CL_INVALID_PROGRAM_EXECUTABLE\n"); exit(-1); 
		case CL_INVALID_KERNEL_NAME: 
			printf("CL_INVALID_KERNEL_NAME\n"); exit(-1); 
		case CL_INVALID_KERNEL_DEFINITION: 
			printf("CL_INVALID_KERNEL_DEFINITION\n"); exit(-1); 
		case CL_INVALID_KERNEL: 
			printf("CL_INVALID_KERNEL\n"); exit(-1); 
		case CL_INVALID_ARG_INDEX:
			printf("CL_INVALID_ARG_INDEX\n"); exit(-1); 
		case CL_INVALID_ARG_VALUE:
			printf("CL_INVALID_ARG_VALUE\n"); exit(-1); 
		case CL_INVALID_ARG_SIZE: 
			printf("CL_INVALID_ARG_SIZE\n"); exit(-1); 
		case CL_INVALID_KERNEL_ARGS: 
			printf("CL_INVALID_KERNEL_ARGS\n"); exit(-1); 
		case CL_INVALID_WORK_DIMENSION: 
			printf("CL_INVALID_WORK_DIMENSION\n"); exit(-1); 
		case CL_INVALID_WORK_GROUP_SIZE: 
			printf("CL_INVALID_WORK_GROUP_SIZE\n"); exit(-1); 
		case CL_INVALID_WORK_ITEM_SIZE: 
			printf("CL_INVALID_WORK_ITEM_SIZE\n"); exit(-1); 
		case CL_INVALID_GLOBAL_OFFSET: 
			printf("CL_INVALID_GLOBAL_OFFSET\n"); exit(-1); 
		case CL_INVALID_EVENT_WAIT_LIST: 
			printf("CL_INVALID_EVENT_WAIT_LIST\n"); exit(-1); 
		case CL_INVALID_EVENT:
			printf("CL_INVALID_EVENT\n"); exit(-1); 
		case CL_INVALID_OPERATION:
			printf("CL_INVALID_OPERATION\n"); exit(-1); 
		case CL_INVALID_GL_OBJECT: 
			printf("CL_INVALID_GL_OBJECT\n"); exit(-1); 
		case CL_INVALID_BUFFER_SIZE: 
			printf("CL_INVALID_BUFFER_SIZE\n"); exit(-1); 
		case CL_INVALID_MIP_LEVEL: 
			printf("CL_INVALID_MIP_LEVEL\n"); exit(-1); 
		case CL_INVALID_GLOBAL_WORK_SIZE: 
			printf("CL_INVALID_GLOBAL_WORK_SIZE\n"); exit(-1); 
		case CL_INVALID_PROPERTY:
			printf("CL_INVALID_PROPERTY\n"); exit(-1); 
		case CL_INVALID_IMAGE_DESCRIPTOR:
			printf("CL_INVALID_IMAGE_DESCRIPTOR\n"); exit(-1); 
		case CL_INVALID_COMPILER_OPTIONS:
			printf("CL_INVALID_COMPILER_OPTIONS\n"); exit(-1); 
		case CL_INVALID_LINKER_OPTIONS: 
			printf("CL_INVALID_LINKER_OPTIONS\n"); exit(-1); 
		case CL_INVALID_DEVICE_PARTITION_COUNT: 
			printf("CL_INVALID_DEVICE_PARTITION_COUNT\n"); exit(-1); 
		}
	}
}
