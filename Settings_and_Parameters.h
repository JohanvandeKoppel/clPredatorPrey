//
//  AridLands.cpp
//
//  Created by Johan Van de Koppel on 03-09-14.
//  Copyright (c) 2014 Johan Van de Koppel. All rights reserved.
//

// Compiler directives
#define ON              1
#define OFF             0

#define Print_All_Devices OFF

#define Device_No       1    // 0: CPU; 1: Intel 4000; 2: Nvidia
#define ProgressBarWidth 45

// Thread block size
#define Block_Size_X	16                // 16
#define Block_Size_Y	16                // 16

// Number of blox
/* I define the Block_Number_ensions of the matrix as product of two numbers
Makes it easier to keep them a multiple of something (16, 32) when using CUDA*/
#define Block_Number_X	64                // 32
#define Block_Number_Y	64                // 32

// Matrix Block_Number_ensions
// (chosen as multiples of the thread block size for simplicity)
#define Grid_Width  (Block_Size_X * Block_Number_X)			// Matrix A width
#define Grid_Height (Block_Size_Y * Block_Number_Y)			// Matrix A height
#define Grid_Size (Grid_Width*Grid_Height)                   // Grid Size

// DIVIDE_INTO(x/y) for integers, used to determine # of blocks/warps etc.
#define DIVIDE_INTO(x,y) (((x) + (y) - 1)/(y))

// Definition of spatial parameters
#define dX			2		//  5     The size of each grid cell in X direction
#define dY			2		//  5     The size of each grid cell in Y direction

// Process parameters            Original value    Explanation and Units
#define DifPrey		1		//  1
#define DifPred		1		//  4

#define A			3.5		//  3.5
#define B			1.2		//  1.2
#define C			4.9		//  4.9 / 6

#define dT	        0.05	// Timestep
#define NumFrames	1000    // The number of time the results are stored
#define	MAX_STORE	(NumFrames+2)	// Some more space than needed
#define EndTime		1000	// The time at which the simulation ends

// Name definitions
#define PREY		101
#define PREDATOR	102

#define HORIZONTAL	201
#define VERTICAL	202

