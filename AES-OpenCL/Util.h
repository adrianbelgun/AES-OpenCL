#ifndef _UTIL_H_
#define _UIL_H_

// ******** General Utility Functions *****************************************
int		GetArgInt   (const int argc, char** argv, const char* sArg);
char*	GetArgString(const int argc, char** argv, const char* sArg);

bool FileExists(const char * filename);
std::string GetFileContents(const char *filename);

double GetTime();


// ******** Cryptography Utility Functions ************************************
void ComputeRoundKeys(unsigned char** roundKeys, int* rounds, size_t size, 
	unsigned char* key);

// ******** OpenCL Utility Functions ******************************************
void PrintPlatformInfo(cl_platform_id);
void PrintDeviceInfo  (cl_device_id);

void CHECK_OPENCL_ERROR(cl_int err);

// ******** Image Utility Functions ******************************************
struct ImageData
{
	size_t columns;
	size_t rows;
	size_t bytes;
	size_t padded_bytes;
	unsigned char* data;
};

ImageData ReadImageFile (char* name);
void WriteImageFile (char* name, ImageData img);

#endif