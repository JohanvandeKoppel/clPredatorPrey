//
//  AridLands main.cpp
//
//  Created by Johan Van de Koppel on 03-09-14.
//  Copyright (c) 2014 Johan Van de Koppel. All rights reserved.
//

#include <stdio.h>
#include <sys/time.h>
#include <iostream>
#include <math.h>

#include "Settings_and_Parameters.h"
#include "Device_Utilities.h"

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define MAX_SOURCE_SIZE (0x100000)

// Forward definitions from functions at the end of this code file
void randomInit(float*, int, int, int);
void blockInit(float*, int, int, int);

////////////////////////////////////////////////////////////////////////////////
// Main program code for Aridlands
////////////////////////////////////////////////////////////////////////////////

int main()
{
    
    /*----------Constant and variable definition------------------------------*/
    
	unsigned int Grid_Memory = sizeof(float) * Grid_Size;
	unsigned int size_storegrid = Grid_Width * Grid_Height * MAX_STORE;
	unsigned int mem_size_storegrid = sizeof(float) * size_storegrid;
    
    /*----------Defining and allocating memeory on host-----------------------*/
    
    // Defining and allocating the memory blocks for P, W, and O on the host (h)
	float * h_Prey = (float *)malloc(Grid_Width*Grid_Height*sizeof(float));
	float * h_Pred = (float *)malloc(Grid_Width*Grid_Height*sizeof(float));
	
    // Defining and allocating storage blocks for P, W, and O on the host (h)
    float * h_store_Prey=(float*) malloc(mem_size_storegrid);
	float * h_store_Pred=(float*) malloc(mem_size_storegrid);
    
    /*----------Initializing the host arrays----------------------------------*/
    
    srand(50);
    
	//blockInit(h_Prey, Grid_Width, Grid_Height, PREY);
	//blockInit(h_Pred, Grid_Width, Grid_Height, PREDATOR);
    
	randomInit(h_Prey, Grid_Width, Grid_Height, PREY);
	randomInit(h_Pred, Grid_Width, Grid_Height, PREDATOR);
    
    /*----------Printing info to the screen ----------------------------------*/

	//system("clear");
    printf("\n");
	printf(" * * * * * * * * * * * * * * * * * * * * * * * * * * * * * \n");
	printf(" * Predator Prey Spirals                                 * \n");
	printf(" * OpenCL implementation : Johan van de Koppel 2014      * \n");
	printf(" * Following a model by Sherrat et al 2001               * \n");
	printf(" * * * * * * * * * * * * * * * * * * * * * * * * * * * * * \n\n");
    
	printf(" Current grid dimensions: %d x %d cells\n\n",
           Grid_Width, Grid_Height);
    
    /*----------Setting up the device and the kernel--------------------------*/
    
    cl_device_id* devices;
    cl_int err;
    
    cl_context context = CreateGPUcontext(devices);
    
    // Print the name of the device that is used
    printf(" Implementing PDE on device %d: ", Device_No);
    print_device_info(devices, (int)Device_No);
    printf("\n");
    
    // Create a command queue on the device
    cl_command_queue command_queue = clCreateCommandQueue(context, devices[Device_No], 0, &err);
    
    /* Create Buffer Objects */
	cl_mem d_Prey = clCreateBuffer(context, CL_MEM_READ_WRITE, Grid_Memory, NULL, &err);
	cl_mem d_Pred = clCreateBuffer(context, CL_MEM_READ_WRITE, Grid_Memory, NULL, &err);
    
	/* Copy input data to the memory buffer */
	err = clEnqueueWriteBuffer(command_queue, d_Prey, CL_TRUE, 0, Grid_Width*Grid_Height*sizeof(float), h_Prey, 0, NULL, NULL);
	err = clEnqueueWriteBuffer(command_queue, d_Pred, CL_TRUE, 0, Grid_Width*Grid_Height*sizeof(float), h_Pred, 0, NULL, NULL);
    
    /*----------Building the PDE kernel---------------------------------------*/
    
    cl_program program = BuildKernelFile("Computing_Kernel.cl", context, &devices[Device_No], &err);
    if (err!=0)  printf(" > Compile Program Error number: %d \n\n", err);
    
    /* Create OpenCL kernel */
    cl_kernel kernel = clCreateKernel(program, "PredPreyKernel", &err);
    if (err!=0) printf(" > Create Kernel Error number: %d \n\n", err);

	/* Set OpenCL kernel arguments */
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&d_Prey);
	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&d_Pred);
    
    /*----------Pre-simulation settings---------------------------------------*/
    
    /* create and start timer */
    struct timeval Time_Measured;
    gettimeofday(&Time_Measured, NULL);
    double Time_Begin=Time_Measured.tv_sec+(Time_Measured.tv_usec/1000000.0);

    /* Progress bar initiation */
    int RealBarWidth=std::min((int)NumFrames,(int)ProgressBarWidth);
    int BarCounter=0;
    float BarThresholds[RealBarWidth];
    for (int i=0;i<RealBarWidth;i++) {BarThresholds[i] = (float)(i+1)/RealBarWidth*NumFrames; };
    
    /* Print the reference bar */
    printf(" Progress: [");
    for (int i=0;i<RealBarWidth;i++) { printf("-"); }
    printf("]\n");
    fprintf(stderr, "           >");
    
    /*----------Kernel parameterization---------------------------------------*/
    
	size_t global_item_size = Grid_Width*Grid_Height;
	size_t local_item_size = Block_Number_X*Block_Number_Y;
    
	for (int Counter=0;Counter<NumFrames;Counter++)
    {
        for (int Runtime=0;Runtime<floor(EndTime/NumFrames/dT);Runtime++)
        {
            /* Execute OpenCL kernel as data parallel */
            err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
                                     &global_item_size, &local_item_size, 0, NULL, NULL);
            if (err!=0) { printf(" > Kernel Error number: %d \n\n", err); exit(-10);}
        }
        
        /* Transfer result to host */
        err  = clEnqueueReadBuffer(command_queue, d_Prey, CL_TRUE, 0, Grid_Width*Grid_Height*sizeof(float), h_Prey, 0, NULL, NULL);
        err |= clEnqueueReadBuffer(command_queue, d_Pred, CL_TRUE, 0, Grid_Width*Grid_Height*sizeof(float), h_Pred, 0, NULL, NULL);

        if (err!=0) printf("Read Buffer Error: %d\n\n", err);
        
        //Store values at this frame.
        memcpy(h_store_Prey+(Counter*Grid_Size),h_Prey,Grid_Memory);
        memcpy(h_store_Pred+(Counter*Grid_Size),h_Pred,Grid_Memory);
        
        // Progress the progress bar if time
        if ((float)(Counter+1)>=BarThresholds[BarCounter]) {
            fprintf(stderr,"*");
            BarCounter = BarCounter+1;}
            
    }
    
    fprintf(stdout,"<\n\n");
    
    /*---------------------Report on time spending----------------------------*/
    
    gettimeofday(&Time_Measured, NULL);
    double Time_End=Time_Measured.tv_sec+(Time_Measured.tv_usec/1000000.0);
	printf(" Processing time: %4.5f (s) \n", Time_End-Time_Begin);
    
    /*---------------------Write to file now----------------------------------*/
    
    // The location of the code is obtain from the __FILE__ macro
    const std::string SourcePath (__FILE__);
    const std::string PathName = SourcePath.substr (0,SourcePath.find_last_of("/")+1);
    const std::string DataPath = PathName + "clPredPrey.dat";
    
	FILE * fp=fopen(DataPath.c_str(),"wb");

    int width_matrix = Grid_Width;
    int height_matrix = Grid_Height;
    int NumStored = NumFrames;
    float Length = dX*(float)Grid_Width;
    int EndTimeVal = EndTime;

	// Storing parameters
	fwrite(&width_matrix,sizeof(int),1,fp);
	fwrite(&height_matrix,sizeof(int),1,fp);
	fwrite(&NumStored,sizeof(int),1,fp);
    fwrite(&Length,sizeof(float),1,fp);
	fwrite(&EndTimeVal,sizeof(int),1,fp);
	
	for(int store_i=0;store_i<NumFrames;store_i++)
    {
		fwrite(&h_store_Prey[store_i*Grid_Size],sizeof(float),Grid_Size,fp);
		fwrite(&h_store_Pred[store_i*Grid_Size],sizeof(float),Grid_Size,fp);
    }
	
	printf(" Simulation results saved! \n\n");
    
	fclose(fp);
    
	/*---------------------Clean up memory------------------------------------*/
	
    // Freeing host space
    free(h_Prey);
	free(h_Pred);
    
	free(h_store_Prey);
	free(h_store_Pred);
 
	// Freeing kernel and block space
	err = clFlush(command_queue);
	err = clFinish(command_queue);
	err = clReleaseKernel(kernel);
	err = clReleaseProgram(program);
	err = clReleaseMemObject(d_Prey);
	err = clReleaseMemObject(d_Pred);
	err = clReleaseCommandQueue(command_queue);
	err = clReleaseContext(context);
    free(devices);
    
    #if defined(__APPLE__) && defined(__MACH__)
        //system("say Simulation finished");
    #endif

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// A set of blocks as initial values that quickly generates spirals
////////////////////////////////////////////////////////////////////////////////

void blockInit(float* data, int x_siz, int y_siz, int type)
{
    int i,j;
    int Mi=100;
    int Mj=100;
    float Ci,Cj;
    
    for(i=0;i<y_siz;i++)
    {
        for(j=0;j<x_siz;j++)
 		{
			//for every element find the correct initial
			//value using the conditions below
			if(i==0||i==y_siz-1||j==0||j==x_siz-1)
				data[i*y_siz+j]=0.0f; // This value for the boundaries
			else
            {
				Ci=((float)pow(i,1.005)/(float)Mi)-floor((float)i/(float)Mi);
                Cj=((float)pow(j,1.005)/(float)Mj)-floor((float)j/(float)Mj);
                
				if(type==PREY)
                {
                    if(Ci>0.5||Cj>0.5)
                    {
                        data[i*y_siz+j]=0.5f;
                    }
                }
				else if(type==PREDATOR)
                {
                    if(Ci>0.97||Cj>0.75)
                    {
                        data[i*y_siz+j]=5.0f;
                    }
                }
                
            }
		}
    }
} // End BlockInit

////////////////////////////////////////////////////////////////////////////////
// Allocates a matrix with random float entries
////////////////////////////////////////////////////////////////////////////////

void randomInit(float* data, int x_siz, int y_siz, int type)
{
	int i,j;
	for(i=0;i<y_siz;i++)
	{
		for(j=0;j<x_siz;j++)
		{
			//for every element find the correct initial
			//value using the conditions below
			if(i==0||i==y_siz-1||j==0||j==x_siz-1)
			    data[i*y_siz+j]=0.0f; // This value for the boundaries
			else
			{
				if(type==PREDATOR)
				{
                    // A randomized initiation here
                    if((rand() / (float)RAND_MAX)<0.0005f)
                        data[i*y_siz+j] = 1.0f;
                    else
                        data[i*y_siz+j] = 0.0f;
				}
				else if(type==PREY)
					data[i*y_siz+j]=(float)1.0f;
                
			}
		}
	}			
    
} // End randomInit
