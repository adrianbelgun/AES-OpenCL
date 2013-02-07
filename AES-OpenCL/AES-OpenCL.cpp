// AES-OpenCL.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

void PrintUsage()
{
	printf("Usage:\n");
	printf("  <program> --mode=benchmark --algo=<ctr|ecb>\n");
	printf("  <program> --mode=<encrypt|decrypt> --algo=<ctr|ecb> --in=<file> --out=<file>\n");
}

int ImageOperationECB(int argc, char** argv, bool bEncrypt = true)
{
	// Parse arguments
	// OpenCL arguments: platform and device
	cl_int err;
		
	int iPlatform  = GetArgInt   (argc, argv, "p");
	int iDevice    = GetArgInt   (argc, argv, "d");
	char* sInFile  = GetArgString(argc, argv, "in");
	char* sOutFile = GetArgString(argc, argv, "out");

	if (sInFile == NULL || sOutFile == NULL || !FileExists(sInFile))
	{
		PrintUsage();
		return -1;
	}

	// Initialize ImageMagick 
	Magick::InitializeMagick(*argv);
	
	ImageData img = ReadImageFile(sInFile);

	// Allocate Host Memory
	unsigned char key[16] = {
		0x2B, 0x7E, 0x15, 0x16, 
		0x28, 0xAE, 0xD2, 0xA6, 
		0xAB, 0xF7, 0x15, 0x88, 
		0x09, 0xCF, 0x4F, 0x3C};
	unsigned char* roundKeys = NULL;
	int rounds = 0;

	ComputeRoundKeys(&roundKeys, &rounds, 16, key);
	
	// Set-up OpenCL Platform
	OCL_Device* pOCL_Device = new OCL_Device(iPlatform, iDevice);
	pOCL_Device->SetBuildOptions("");
	pOCL_Device->PrintInfo();

	// Set up OpenCL 
	cl_kernel Kernel = pOCL_Device->GetKernel("aes-kernel.cl",
		bEncrypt ? "AES_ECB_Encrypt" : "AES_ECB_Decrypt");

	// Allocate Device Memory
	cl_mem d_A = pOCL_Device->DeviceMalloc(0, img.padded_bytes);
	cl_mem d_B = pOCL_Device->DeviceMalloc(1, img.padded_bytes);
	cl_mem d_C = pOCL_Device->DeviceMalloc(2, rounds * 16);
	
	// Copy Image to Device
	pOCL_Device->CopyBufferToDevice(img.data, 0, img.padded_bytes);	
	
	// Keys
	pOCL_Device->CopyBufferToDevice(roundKeys, 2, rounds * 16);
	

	// Set Kernel Arguments
	cl_int _num = img.padded_bytes / 16;
	err = clSetKernelArg(Kernel, 0, sizeof(cl_mem), &d_A);   
	CHECK_OPENCL_ERROR(err);
	err = clSetKernelArg(Kernel, 1, sizeof(cl_mem), &d_B);    
	CHECK_OPENCL_ERROR(err);
	err = clSetKernelArg(Kernel, 2, sizeof(cl_mem), &d_C);    
	CHECK_OPENCL_ERROR(err);
	err = clSetKernelArg(Kernel, 3, sizeof(cl_int), &rounds); 
	CHECK_OPENCL_ERROR(err);
	err = clSetKernelArg(Kernel, 4, sizeof(cl_int), &_num);   
	CHECK_OPENCL_ERROR(err);
	
	
	// Wait for previous action to finish
	err = clFinish(pOCL_Device->GetQueue());
	CHECK_OPENCL_ERROR(err);
	
	size_t off = 0;
	size_t num = img.padded_bytes / 16;
	size_t threads = 256;

	// Run the kernel
	err = clEnqueueNDRangeKernel(pOCL_Device->GetQueue(), 
		Kernel, 1, NULL, &num, &threads, 0, NULL, NULL);
	CHECK_OPENCL_ERROR(err);
	
	
	// Wait for kernel to finish
	err = clFinish(pOCL_Device->GetQueue());
	CHECK_OPENCL_ERROR(err);


	// Copy Data From Device
	pOCL_Device->CopyBufferToHost  (img.data, 1, img.padded_bytes);
	
	// Free resources
	delete pOCL_Device;
	
	delete[] roundKeys;
	
	// Write Output data
	WriteImageFile(sOutFile, img);

	free(img.data);

	return 0;
}

// Decrypt is the same as Encrypt
int ImageOperationCTR(int argc, char** argv)
{
	// Parse arguments
	// OpenCL arguments: platform and device
	cl_int err;
		
	int iPlatform  = GetArgInt   (argc, argv, "p");
	int iDevice    = GetArgInt   (argc, argv, "d");
	char* sInFile  = GetArgString(argc, argv, "in");
	char* sOutFile = GetArgString(argc, argv, "out");

	if (sInFile == NULL || sOutFile == NULL || !FileExists(sInFile))
	{
		PrintUsage();
		return -1;
	}

	// Initialize ImageMagick 
	Magick::InitializeMagick(*argv);
	
	ImageData img = ReadImageFile(sInFile);

	// Allocate Host Memory
	unsigned char key[16] = {
		0x2B, 0x7E, 0x15, 0x16, 
		0x28, 0xAE, 0xD2, 0xA6, 
		0xAB, 0xF7, 0x15, 0x88, 
		0x09, 0xCF, 0x4F, 0x3C};
	unsigned char* roundKeys = NULL;
	int rounds = 0;

	unsigned char nonce[12] = {
		0x0, 0x1, 0x2, 0x3,
		0x4, 0x5, 0x6, 0x7,
		0x8, 0x9, 0xA, 0xB};
	/*
	srand(time(NULL));
	for (int i = 0; i < 12; i++)
		nonce[i] = rand() % 256;
	*/ 

	ComputeRoundKeys(&roundKeys, &rounds, 16, key);
	
	// Set-up OpenCL Platform
	OCL_Device* pOCL_Device = new OCL_Device(iPlatform, iDevice);
	pOCL_Device->SetBuildOptions("");
	pOCL_Device->PrintInfo();

	// Set up OpenCL 
	cl_kernel Kernel = pOCL_Device->GetKernel("aes-kernel.cl", "AES_CTR_Encrypt");

	// Allocate Device Memory
	cl_mem d_A = pOCL_Device->DeviceMalloc(0, img.padded_bytes);
	cl_mem d_B = pOCL_Device->DeviceMalloc(1, img.padded_bytes);
	cl_mem d_C = pOCL_Device->DeviceMalloc(2, rounds * 16);
	cl_mem d_D = pOCL_Device->DeviceMalloc(3, 12);

	// Copy Image to Device
	pOCL_Device->CopyBufferToDevice(img.data, 0, img.padded_bytes);	
	
	// Keys
	pOCL_Device->CopyBufferToDevice(roundKeys, 2, rounds * 16);

	// Nonce
	pOCL_Device->CopyBufferToDevice(nonce, 3, 12);
	
	// Set Kernel Arguments
	cl_int _num = img.padded_bytes / 16;
	err = clSetKernelArg(Kernel, 0, sizeof(cl_mem), &d_A);   
	CHECK_OPENCL_ERROR(err);
	err = clSetKernelArg(Kernel, 1, sizeof(cl_mem), &d_B);    
	CHECK_OPENCL_ERROR(err);
	err = clSetKernelArg(Kernel, 2, sizeof(cl_mem), &d_C);    
	CHECK_OPENCL_ERROR(err);
	err = clSetKernelArg(Kernel, 3, sizeof(cl_int), &rounds); 
	CHECK_OPENCL_ERROR(err);
	err = clSetKernelArg(Kernel, 4, sizeof(cl_int), &_num);   
	CHECK_OPENCL_ERROR(err);
	err = clSetKernelArg(Kernel, 5, sizeof(cl_mem), &d_D);    
	CHECK_OPENCL_ERROR(err);
		
	// Wait for previous action to finish
	err = clFinish(pOCL_Device->GetQueue());
	CHECK_OPENCL_ERROR(err);
	
	size_t off = 0;
	size_t num = img.padded_bytes / 16;
	size_t threads = 256;

	// Run the kernel
	err = clEnqueueNDRangeKernel(pOCL_Device->GetQueue(), 
		Kernel, 1, NULL, &num, &threads, 0, NULL, NULL);
	CHECK_OPENCL_ERROR(err);
		
	// Wait for kernel to finish
	err = clFinish(pOCL_Device->GetQueue());
	CHECK_OPENCL_ERROR(err);

	// Copy Data From Device
	pOCL_Device->CopyBufferToHost  (img.data, 1, img.padded_bytes);
	
	// Free resources
	delete pOCL_Device;
	
	delete[] roundKeys;
	
	// Write Output data
	WriteImageFile(sOutFile, img);

	free(img.data);

	return 0;
}

int test1(int argc, char** argv)
{
	// Parse arguments
	// OpenCL arguments: platform and device
	cl_int err;
		
	int iPlatform = GetArgInt(argc, argv, "p");
	int iDevice   = GetArgInt(argc, argv, "d");
	char* sFileName = GetArgString(argc, argv, "n");

	// Allocate Host Memory
	unsigned char pattern[16] =
	{
		0x32, 0x43, 0xF6, 0xA8, 
		0x88, 0x5A, 0x30, 0x8D,
		0x31, 0x31, 0x98, 0xA2,
		0xE0, 0x37, 0x07, 0x34};
	unsigned char data[16*256];
	for (int i = 0; i < 256; i++)
		for (int k; k < 16; k++)
			data[i*16 + k] = pattern[k];

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			printf("%2X ", data[i + j*4]);
		printf("\n");
	}
	printf("\n");		

	unsigned char key[16] = {
		0x2B, 0x7E, 0x15, 0x16, 
		0x28, 0xAE, 0xD2, 0xA6, 
		0xAB, 0xF7, 0x15, 0x88, 
		0x09, 0xCF, 0x4F, 0x3C};
	unsigned char* roundKeys = NULL;
	int rounds = 0;

	ComputeRoundKeys(&roundKeys, &rounds, 16, key);
	
	// Set-up OpenCL Platform
	OCL_Device* pOCL_Device = new OCL_Device(iPlatform, iDevice);
	pOCL_Device->SetBuildOptions("");
	pOCL_Device->PrintInfo();

	// Set up OpenCL 
	cl_kernel kernel = pOCL_Device->GetKernel("aes-kernel.cl", "AES_ECB_Encypt4");

	// Allocate Device Memory
	cl_mem d_A = pOCL_Device->DeviceMalloc(0, 16);
	cl_mem d_B = pOCL_Device->DeviceMalloc(1, 16);
	cl_mem d_C = pOCL_Device->DeviceMalloc(2, rounds * 16);
	
	// Copy Image to Device
	pOCL_Device->CopyBufferToDevice(data, 0, 16);	
	
	// Keys
	pOCL_Device->CopyBufferToDevice(roundKeys, 2, rounds * 16);
	

	// Set Kernel Arguments
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_A); CHECK_OPENCL_ERROR(err);
	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_B); CHECK_OPENCL_ERROR(err);
	err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_C); CHECK_OPENCL_ERROR(err);
	err = clSetKernelArg(kernel, 3, sizeof(cl_int), &rounds);   CHECK_OPENCL_ERROR(err);
	cl_int _num = 1;
	err = clSetKernelArg(kernel, 4, sizeof(cl_int), &_num);   CHECK_OPENCL_ERROR(err);
	
	// Wait for previous action to finish
	err = clFinish(pOCL_Device->GetQueue());
	CHECK_OPENCL_ERROR(err);

	double seconds = GetTime();
	// Run the kernel
	size_t off = 0;
	size_t num = 256;
	size_t threads = 256;
	err = clEnqueueNDRangeKernel(pOCL_Device->GetQueue(), kernel, 1, NULL, &num, &threads, 0, NULL, NULL);
	CHECK_OPENCL_ERROR(err);

	// Wait for kernel to finish
	err = clFinish(pOCL_Device->GetQueue());
	CHECK_OPENCL_ERROR(err);
	seconds = GetTime() - seconds;
	printf("Elapsed Time: %f s (%f MiB/s)\n" , seconds, 16 /seconds * 10.f / 1024.f / 1024.f);

	// Copy Data From Device
	pOCL_Device->CopyBufferToHost  (data, 1, 16);
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			printf("%2X ", data[i + j*4]);
		printf("\n");
	}
	printf("\n");		
	
	// Free resources
	delete pOCL_Device;
	
	delete[] roundKeys;


	//write test data
	//WriteImageFile("tux2.jpg", img);

	//free(img.data);

	return 0;
}

int benchmark_ecb(int argc, char** argv)
{
	// Parse arguments
	// OpenCL arguments: platform and device
	cl_int err;
	int count = 100;

		
	int iPlatform = GetArgInt(argc, argv, "p");
	int iDevice   = GetArgInt(argc, argv, "d");
	
	// Set-up  Encryption keys
	unsigned char key[16] = {
		0x2B, 0x7E, 0x15, 0x16, 
		0x28, 0xAE, 0xD2, 0xA6, 
		0xAB, 0xF7, 0x15, 0x88, 
		0x09, 0xCF, 0x4F, 0x3C};
	unsigned char* roundKeys = NULL;
	int rounds = 0;
	ComputeRoundKeys(&roundKeys, &rounds, 16, key);
	
	// Set-up OpenCL Platform
	OCL_Device* pOCL_Device = new OCL_Device(iPlatform, iDevice);
	pOCL_Device->SetBuildOptions("");
	pOCL_Device->PrintInfo();

	// Set up OpenCL 
	cl_kernel EncryptionKernel = 
		pOCL_Device->GetKernel("aes-kernel.cl", "AES_ECB_Encrypt3");
	cl_kernel DecryptionKernel = 
		pOCL_Device->GetKernel("aes-kernel.cl", "AES_ECB_Decrypt3");

	size_t MinSize = 16;		 // 16 B = 128 bits
	size_t MaxSize = 512 << 20;  // 512 MiB.

	// keys
	cl_mem d_C = pOCL_Device->DeviceMalloc(2, rounds * 16);
	pOCL_Device->CopyBufferToDevice(roundKeys, 2, rounds * 16);

	printf("\n");
	printf("Time is reported for %d passes.\n", count);
	printf("\n");
	
	printf("     MiB    , Encryption Speed (MiB/s), Encryption Time (s), Decryption Speed (MiB/s), Decryption Time (s)\n");

	for (size_t size = MinSize; size <= MaxSize; size *= 2)
	{
		printf("%12.8f, ", ((double)size) / 1024 / 1024);

		// Allocate Device Memory
		cl_mem d_A = pOCL_Device->DeviceMalloc(0, size);
		cl_mem d_B = pOCL_Device->DeviceMalloc(1, size);

		// Allocate Host Memory
		char* h_A  = new char[size];
		char* h_B  = new char[size];

		// Fill Host Memory

		for (size_t i = 0; i < size; i++)
		{
			h_A[i] = i % 27;
		}
	
		// Copy Data to Device
		pOCL_Device->CopyBufferToDevice(h_A, 0, size);	
		pOCL_Device->CopyBufferToDevice(h_A, 1, size);	

		// Set Kernel Arguments
		// Encrypt kernel
		cl_int _num = size / 16;
		err = clSetKernelArg(EncryptionKernel, 0, sizeof(cl_mem), &d_A); 
		CHECK_OPENCL_ERROR(err);
		err = clSetKernelArg(EncryptionKernel, 1, sizeof(cl_mem), &d_B);
		CHECK_OPENCL_ERROR(err);
		err = clSetKernelArg(EncryptionKernel, 2, sizeof(cl_mem), &d_C); 
		CHECK_OPENCL_ERROR(err);
		err = clSetKernelArg(EncryptionKernel, 3, sizeof(cl_int), &rounds);   
		CHECK_OPENCL_ERROR(err);
		err = clSetKernelArg(EncryptionKernel, 4, sizeof(cl_int), &_num); 
		CHECK_OPENCL_ERROR(err);
	
		// Decrypt Kernel
		err = clSetKernelArg(DecryptionKernel, 0, sizeof(cl_mem), &d_B); 
		CHECK_OPENCL_ERROR(err);
		err = clSetKernelArg(DecryptionKernel, 1, sizeof(cl_mem), &d_A);
		CHECK_OPENCL_ERROR(err);
		err = clSetKernelArg(DecryptionKernel, 2, sizeof(cl_mem), &d_C);
		CHECK_OPENCL_ERROR(err);
		err = clSetKernelArg(DecryptionKernel, 3, sizeof(cl_int), &rounds);
		CHECK_OPENCL_ERROR(err);
		err = clSetKernelArg(DecryptionKernel, 4, sizeof(cl_int), &_num); 
		CHECK_OPENCL_ERROR(err);
	
		// Wait for previous action to finish
		err = clFinish(pOCL_Device->GetQueue());
		CHECK_OPENCL_ERROR(err);
			
		size_t off = 0;
		size_t num = (((size / 16) + 255) / 256) * 256;
		size_t threads = 256;
		
		// Run the encryption kernel
		double seconds = GetTime();
		for (int i = 0; i < count; i++)
			err = clEnqueueNDRangeKernel(pOCL_Device->GetQueue(), 
			EncryptionKernel, 1, NULL, &num, &threads, 0, NULL, NULL);
		CHECK_OPENCL_ERROR(err);
		err = clFinish(pOCL_Device->GetQueue());
		CHECK_OPENCL_ERROR(err);
		seconds = GetTime() - seconds;
		printf("     %12.4f,    ", size / seconds / 1024.f / 1024.f * count);
		printf("     %12.4f,    ", seconds);

		// Override Input Buffer
		err = clEnqueueCopyBuffer(pOCL_Device->GetQueue(), d_B, d_A, 0, 0, 
			size, 0, NULL, NULL);
		CHECK_OPENCL_ERROR(err);
		err = clFinish(pOCL_Device->GetQueue());
		CHECK_OPENCL_ERROR(err);

		// Run the decryption kernel
		seconds = GetTime();
		for (int i = 0; i < count; i++)
			err = clEnqueueNDRangeKernel(pOCL_Device->GetQueue(), 
			DecryptionKernel, 1, NULL, &num, &threads, 0, NULL, NULL);
		CHECK_OPENCL_ERROR(err);
		err = clFinish(pOCL_Device->GetQueue());
		CHECK_OPENCL_ERROR(err);
		seconds = GetTime() - seconds;
		printf("     %12.4f,    ", size / seconds / 1024.f / 1024.f * count);
		printf("     %12.4f,    ", seconds);
		

		// Copy Data From Device
		pOCL_Device->CopyBufferToHost  (h_B, 0, size);
		
		// Verify Data
		bool passed = true;
		for (size_t i = 0; i < size; i++)
		{
			if (h_A[i] != h_B[i])
			{
				passed = false;
				printf("\n");
				printf("Encountered an error when running the benchmark with size %d.\n",
					size);
				printf("At element %d: %d != %d.\n", i, h_A[i], h_B[i]);
				printf("\n");
				break;
			}
		}
		if (passed)
			printf("        passed  ");
		else
			printf("        failed  ");
		printf("\n");
	}
	// Free resources
	delete pOCL_Device;
	
	delete[] roundKeys;

	return 0;
}

int benchmark_ctr(int argc, char** argv)
{
	// Parse arguments
	// OpenCL arguments: platform and device
	cl_int err;
	int count = 100;

		
	int iPlatform = GetArgInt(argc, argv, "p");
	int iDevice   = GetArgInt(argc, argv, "d");
	
	// Set-up  Encryption keys
	unsigned char key[16] = {
		0x2B, 0x7E, 0x15, 0x16, 
		0x28, 0xAE, 0xD2, 0xA6, 
		0xAB, 0xF7, 0x15, 0x88, 
		0x09, 0xCF, 0x4F, 0x3C};
	unsigned char nonce[12];
	srand(time(NULL));
	for (int i = 0; i < 12; i++)
		nonce[i] = rand() % 256;
	unsigned char* roundKeys = NULL;
	int rounds = 0;
	ComputeRoundKeys(&roundKeys, &rounds, 16, key);
	
	// Set-up OpenCL Platform
	OCL_Device* pOCL_Device = new OCL_Device(iPlatform, iDevice);
	pOCL_Device->SetBuildOptions("");
	pOCL_Device->PrintInfo();

	// Set up OpenCL 
	cl_kernel EncryptionKernel = pOCL_Device->GetKernel("aes-kernel.cl", 
		"AES_CTR_Encrypt");

	size_t MinSize = 16;		 // 16 B = 128 bits
	size_t MaxSize = 512 << 20;  // 512 MiB.

	// keys
	cl_mem d_C = pOCL_Device->DeviceMalloc(2, rounds * 16);
	pOCL_Device->CopyBufferToDevice(roundKeys, 2, rounds * 16);
	
	// nonce
	cl_mem d_D = pOCL_Device->DeviceMalloc(3, 12);
	pOCL_Device->CopyBufferToDevice(nonce, 3, 12);

	printf("\n");
	printf("Time is reported for %d passes.\n", count);
	printf("\n");
	
	printf("     MiB    , Encryption Speed (MiB/s), Encryption Time (s), Decryption Speed (MiB/s), Decryption Time (s)\n");

	for (size_t size = MinSize; size <= MaxSize; size *= 2)
	{
		printf("%12.8f, ", ((double)size) / 1024 / 1024);

		// Allocate Device Memory
		cl_mem d_A = pOCL_Device->DeviceMalloc(0, size);
		cl_mem d_B = pOCL_Device->DeviceMalloc(1, size);

		// Allocate Host Memory
		char* h_A  = new char[size];
		char* h_B  = new char[size];

		// Fill Host Memory

		for (size_t i = 0; i < size; i++)
		{
			h_A[i] = i % 27;
		}
	
		// Copy Data to Device
		pOCL_Device->CopyBufferToDevice(h_A, 0, size);	
		pOCL_Device->CopyBufferToDevice(h_A, 1, size);	
		// just to ensure that both buffers are on the device

		// Set Kernel Arguments
		// Encrypt kernel
		cl_int _num = size / 16;
		err = clSetKernelArg(EncryptionKernel, 0, sizeof(cl_mem), &d_A);
		CHECK_OPENCL_ERROR(err);
		err = clSetKernelArg(EncryptionKernel, 1, sizeof(cl_mem), &d_B);
		CHECK_OPENCL_ERROR(err);
		err = clSetKernelArg(EncryptionKernel, 2, sizeof(cl_mem), &d_C); 
		CHECK_OPENCL_ERROR(err);
		err = clSetKernelArg(EncryptionKernel, 3, sizeof(cl_int), &rounds);  
		CHECK_OPENCL_ERROR(err);
		err = clSetKernelArg(EncryptionKernel, 4, sizeof(cl_int), &_num);
		CHECK_OPENCL_ERROR(err);
		err = clSetKernelArg(EncryptionKernel, 5, sizeof(cl_mem), &d_D);  
		CHECK_OPENCL_ERROR(err);
	
		// Wait for previous action to finish
		err = clFinish(pOCL_Device->GetQueue());
		CHECK_OPENCL_ERROR(err);
			
		size_t off = 0;
		size_t num = (((size / 16) + 255) / 256) * 256;
		size_t threads = 256;
		
		// Run the encryption kernel
		double seconds = GetTime();
		for (int i = 0; i < count; i++)
			err = clEnqueueNDRangeKernel(pOCL_Device->GetQueue(), 
			EncryptionKernel, 1, NULL, &num, &threads, 0, NULL, NULL);
		CHECK_OPENCL_ERROR(err);
		err = clFinish(pOCL_Device->GetQueue());
		CHECK_OPENCL_ERROR(err);
		seconds = GetTime() - seconds;

		printf("     %12.4f,    ", size / seconds / 1024.f / 1024.f * count);
		printf("     %12.4f,    ", seconds);

		// Override Input Buffer
		err = clEnqueueCopyBuffer(pOCL_Device->GetQueue(), d_B, d_A, 0, 0, 
			size, 0, NULL, NULL);
		CHECK_OPENCL_ERROR(err);
		err = clFinish(pOCL_Device->GetQueue());
		CHECK_OPENCL_ERROR(err);

		
		err = clSetKernelArg(EncryptionKernel, 1, sizeof(cl_mem), &d_A);
		CHECK_OPENCL_ERROR(err);
		err = clSetKernelArg(EncryptionKernel, 0, sizeof(cl_mem), &d_B);
		CHECK_OPENCL_ERROR(err);

		// Run the decryption kernel
		seconds = GetTime();
		for (int i = 0; i < count; i++)
			err = clEnqueueNDRangeKernel(pOCL_Device->GetQueue(), 
			EncryptionKernel, 1, NULL, &num, &threads, 0, NULL, NULL);
		CHECK_OPENCL_ERROR(err);
		err = clFinish(pOCL_Device->GetQueue());
		CHECK_OPENCL_ERROR(err);
		seconds = GetTime() - seconds;
		printf("     %12.4f,    ", size / seconds / 1024.f / 1024.f * count);
		printf("     %12.4f,    ", seconds);
		

		// Copy Data From Device
		pOCL_Device->CopyBufferToHost  (h_B, 0, size);
		
		// Verify Data
		bool passed = true;
		for (size_t i = 0; i < size; i++)
		{
			if (h_A[i] != h_B[i])
			{
				passed = false;
				printf("\n");
				printf("Encountered an error when running the benchmark with size %d.\n", 
					size);
				printf("At element %d: %d != %d.\n", i, h_A[i], h_B[i]);
				printf("\n");
				break;
			}
		}
		if (passed)
			printf("        passed  ");
		else
			printf("        failed  ");
		printf("\n");
	}
	// Free resources
	delete pOCL_Device;
	
	delete[] roundKeys;

	return 0;
}

int main(int argc, char** argv)
{
	char* mode = GetArgString(argc, argv, "mode");
	char* algo = GetArgString(argc, argv, "algo");

	if (mode == NULL || algo == NULL)
	{
		PrintUsage();
		return -1;
	}

	int bsMode = 0;
	if (strcmp(mode, "benchmark") == 0)
		bsMode = 0x1;
	else if (strcmp(mode, "encrypt"  ) == 0)
		bsMode = 0x2;
	else if (strcmp(mode, "decrypt"  ) == 0)
		bsMode = 0x4;

	int bsAlgo = 0;	
	if (strcmp(algo, "ecb") == 0)
		bsAlgo = 0x1;
	else if (strcmp(algo, "ctr") == 0)
		bsAlgo = 0x2;
	
	if (bsMode == 0 || bsAlgo == 0)
	{
		PrintUsage();
		return -1;
	}

	if (bsMode == 0x1)
	{
		if (bsAlgo == 0x1) // Benckmark
			benchmark_ecb(argc, argv);
		else if (bsAlgo == 0x2) 
			benchmark_ctr(argc, argv);
	} 
	else if (bsMode == 0x2) // Encrypt
	{
		if (bsAlgo == 0x1) 
			ImageOperationECB(argc, argv, true);
		else if (bsAlgo == 0x2) 
			ImageOperationCTR(argc, argv);
	}
	else if (bsMode == 0x4) // Decrypt
	{
		if (bsAlgo == 0x1) 
			ImageOperationECB(argc, argv, false);
		else if (bsAlgo == 0x2) 
			ImageOperationCTR(argc, argv);
	}
	else
	{	
		PrintUsage();
		return -1;
	}

	return 0;
}

