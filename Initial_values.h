//
//  Initial_values.cpp
//  clMussels
//
//  Created by Johan Van de Koppel on 09/09/15.
//  Copyright (c) 2015 Johan Van de Koppel. All rights reserved.
//

#include <stdio.h>

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

////////////////////////////////////////////////////////////////////////////////
// Prints the model name and additional info
////////////////////////////////////////////////////////////////////////////////

void Print_Label()
{
    //system("clear");
    printf("\n");
    printf(" * * * * * * * * * * * * * * * * * * * * * * * * * * * * * \n");
    printf(" * Predator Prey Spirals                                 * \n");
    printf(" * OpenCL implementation : Johan van de Koppel 2014      * \n");
    printf(" * Following a model by Sherrat et al 2001               * \n");
    printf(" * * * * * * * * * * * * * * * * * * * * * * * * * * * * * \n\n");
    
} // Print_Label
