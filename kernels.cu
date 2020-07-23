#include "kernels.h"
#include <FreeImage.h>
#include <sstream>
#include "BatchLoad.h"
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <iostream>

/*
	Function name - distance
	arguments - x , y and z coodinates, counter to extract coordinates of control point
				the structure object which carries the coordinates
*/
__device__ uint16_t distance(int z,int y,int x,int counter, uint16_t* data)
{
	int cpx = (int)(data[3*counter+0]);
    int cpy = (int)data[3*counter+1];
    int cpz = (int)data[3*counter+2];
    int diff_x = powf((cpx-x),2);
    int diff_y = powf((cpy-y),2);
    int diff_z = powf((cpz-z),2);
    float displacement = (float) sqrtf(diff_x+diff_y+diff_z);
    return (uint16_t) displacement;
}

// sorting algorithm Although not the best technique
// This saves further function calls as in quicksort for partitioning
__device__ void selectionSort(uint16_t arr[], int high)
{
    int i, j, min, temp;
   	for (i = 0; i < high - 1; i++) {
      min = i;
      for (j = i + 1; j < high; j++)
      if (arr[j] < arr[min])
      min = j;
      temp = arr[i];
      arr[i] = arr[min];
      arr[min] = temp;
   }
}


__global__ void kernelcode(int l,int w,int h,uint16_t** GPUOutput,int nc,uint16_t* inputData,pc::Options* opts)
{
	int x =  threadIdx.x + blockIdx.x * blockDim.x;
    int y = (blockIdx.y * blockDim.y) + threadIdx.y;
    int z = blockIdx.z;
    //const int arrsize = opts->numControlPoints;
    uint16_t dist[15]={};
    int index;
    // condition to check if more threads are spawned then execution should be restricted
	if ((x < l) && (y < w) && (z < h)) 
    {
		for(int numCtrlP = 0;numCtrlP<=opts->numControlPoints;numCtrlP++)
		{
			dist[numCtrlP] = distance(z,y,x,numCtrlP,inputData);
		}
		selectionSort(dist,15);
        index = (y * w * 3) + x * 3; 
        int red = (dist[1]/opts->dmax)*255;
        int blue = (dist[1]/opts->dmax)*255;
        int green = (dist[1]/opts->dmax)*255;
        //assigning color as per index
        GPUOutput[z][index + 0] = (int) red;
        GPUOutput[z][index + 1] = (int) green;
        GPUOutput[z][index + 2] = (int) blue;
	}
}

// there is no return values from cuda functions
// this is the only way to debug
#define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort=true)
{
   if (code != cudaSuccess) 
   {
      fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
      if (abort) exit(code);
   }
}

// extracting the coordinates of the control points
uint16_t* convert_to_string(int* size_out,pc::InputData* ob)
{
    *size_out = 6*ob->controlPositions.size();
    uint16_t* ptr = (uint16_t*) malloc(*size_out);
    
    for(int i = 0; i < ob->controlPositions.size(); i++)
    {
        int offset = i*3;
        ptr[offset+0] = (uint16_t) (ob->controlPositions[i].x);
        ptr[offset+1] = (uint16_t) (ob->controlPositions[i].y);
        ptr[offset+2] = (uint16_t) (ob->controlPositions[i].z);
    }

    return ptr;
}

void runGPUVariant(pc::InputData* obj,pc::Options* opts)
{
    // call dummyKernel1 ?!
    //std::cout<<"runGPUVariant called"<<std::endl;
    std::string outputFile = "outputgpu/plane_z";
    pc::Options* GPUoptions = 0;
    uint16_t* GPUinputData = 0;
    int cpSize = 0;
    uint16_t dimX;
    uint16_t dimY;
    uint16_t dimZ;
    int tcount;
    uint16_t* inputDataToString = convert_to_string(&cpSize,obj);
    cudaMalloc((void**)&GPUoptions, 1000);
    cudaMalloc((void**)&GPUinputData, cpSize);
    cudaMemcpy(GPUoptions, opts, 1000, cudaMemcpyHostToDevice);
    cudaMemcpy(GPUinputData, inputDataToString, cpSize, cudaMemcpyHostToDevice);
    //int hieght = opts->z;

    //dim3 grid(1,1,1);
    //dim3 block(2,2,2);
    uint16_t** OutputImg = (uint16_t**) malloc(opts->z * sizeof(uint16_t*));
    uint16_t** GPUOutputImg  = NULL;
    std::cout<<"runGPUVariant cudaMemcpy"<<std::endl;
    //invalid argument error
    cudaMalloc((void**)&GPUOutputImg, opts->z * sizeof(BYTE*));
    for(int i = 0; i < opts->z; i++){cudaMalloc((void**)&OutputImg[i],opts->x*opts->y*3*sizeof(int));}
    gpuErrchk(cudaMemcpy(GPUOutputImg, OutputImg, opts->z * sizeof(uint16_t*), cudaMemcpyHostToDevice));
    
    struct cudaDeviceProp properties;
	cudaGetDeviceProperties(&properties, 0);
	std::cout<<"using "<<properties.multiProcessorCount<<" multiprocessors"<<std::endl;
	std::cout<<"max threads per processor: "<<properties.maxThreadsPerMultiProcessor<<std::endl;
    std::cout<<"@@@ Launching Kernel @@@@"<<std::endl;
    if(opts->mode == pc::Mode::GPUVersion2)
    {
    	dimX = (opts->x / 10);
    	dimY = (opts->y / 10);
    	dimZ = opts->z;
    	const dim3 threadsPerBlock(10,10,1);
    	const dim3 blocksPerGrid(dimX,dimY,dimZ);
    
    	kernelcode <<< blocksPerGrid, threadsPerBlock >>> (opts->x,opts->y,opts->z,GPUOutputImg,opts->numControlPoints,GPUinputData,GPUoptions);
    	gpuErrchk(cudaDeviceSynchronize());

	
    }
    else
    {
    	tcount = sqrt(properties.maxThreadsPerBlock);
    	dimX =  opts->x / tcount;
    	dimY = opts->y / tcount;
    	dimZ = opts->z;
    	const dim3 threadsPerBlock(tcount,tcount,1);
    	const dim3 blocksPerGrid(dimX,dimY,dimZ);
    	kernelcode <<< blocksPerGrid, threadsPerBlock >>> (opts->x,opts->y,opts->z,GPUOutputImg,opts->numControlPoints,GPUinputData,GPUoptions);
		gpuErrchk(cudaDeviceSynchronize());

    }
	std::cout<<"@@@ CUDA Kernel processing @@@@"<<std::endl;
	cudaError_t err = cudaGetLastError();        // Get error code

   	if ( err != cudaSuccess )
   	{
      printf("CUDA Error: %s\n", cudaGetErrorString(err));
      exit(-1);
   	}
   	//idea is to keep z constant and calculate distance of a pixel with remaining control points
    for(int z =0;z<opts->z;z++)
    {
    	//std::cout<<"@@@ Screening z axis @@@@"<<std::endl;
    		
    	FIBITMAP* gpuOutputImage = FreeImage_AllocateT(FIT_BITMAP, opts->x, opts->y, 24);
        BYTE* CPUImageptr = FreeImage_GetBits(gpuOutputImage);

        // Copies calculated image from GPU memory to the FIBITMAP Object and clean up
        gpuErrchk(cudaMemcpy(CPUImageptr, OutputImg[z], opts->x * opts->y * sizeof(BYTE) * 3, cudaMemcpyDeviceToHost));
        gpuErrchk(cudaFree(OutputImg[z]));
        // will be needed for ffmpeg
        std::string name = "outputgpu/plane_z";
	    if(z<10)
	    {
	        name = name+"00"+std::to_string(z)+".png";
	    }
	    else if(z<100)
	    {
	        name = name+"0"+std::to_string(z)+".png";
	    }
	    else
	    {
	        name = name+std::to_string(z)+".png";
	    }
	    std::cout<<" Screened z axis @@@@"<<z<<std::endl;
    	bool returnvalue = GenericWriter(gpuOutputImage, name.c_str(), PNG_DEFAULT);
    	std::cout<<" Returned value is "<<returnvalue<<std::endl;
    	FreeImage_Unload(gpuOutputImage);


    }
    err = cudaGetLastError();        // Get error code

   	if ( err != cudaSuccess )
   	{
      printf("CUDA Error: %s\n", cudaGetErrorString(err));
      exit(-1);
   	}
}
