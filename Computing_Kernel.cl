#include "Settings_and_Parameters.h"

////////////////////////////////////////////////////////////////////////////////
// Laplacation operator definition, to calculate diffusive fluxes
////////////////////////////////////////////////////////////////////////////////

float LaplacianXY(__global float* pop, int row, int column)
{
    float retval;
    int current, left, right, top, bottom;
    float dx = dX;
    float dy = dY;
    
    current=row * Grid_Width + column;
    left=row * Grid_Width + column-1;
    right=row * Grid_Width + column+1;
    top=(row-1) * Grid_Width + column;
    bottom=(row+1) * Grid_Width + column;
    
    retval = ( (( pop[current] - pop[left] )/dx )
              -(( pop[right]   - pop[current] )/dx )) / dx +
             ( (( pop[current] - pop[top] )/dy  )
              -(( pop[bottom]  - pop[current] )/dy ) ) / dy;
    
    return retval;
}

////////////////////////////////////////////////////////////////////////////////
// Simulation kernel
////////////////////////////////////////////////////////////////////////////////

__kernel void PredPreyKernel (__global float* Prey, __global float* Pred)
{
	
 	float d2Preydxy2,d2Preddxy2;
	float drPrey,drPred;
    
    size_t current = get_global_id(0);
	
	int row		= floor((float)current/(float)Grid_Width);
    int column	= current%Grid_Width; // The modulo of current and Width_Grid
    
	if (row > 0 && row < Grid_Width-1 && column > 0 && column < Grid_Width-1)
    {
		//calcualte diffusions for predator and prey in X and Y directions
		// update the current grid values
        
		//Now calculating terms for the Prey Matrix
		d2Preydxy2 =  -DifPrey * LaplacianXY(Prey, row, column);
		drPrey = Prey[current] * (1.0f - Prey[current]) - C/(1.0f + C*Prey[current]) * Prey[current] * Pred[current];
        
		//Now calculating terms for the Predator Matrix
		d2Preddxy2 =  -DifPred * LaplacianXY(Pred, row, column);
		drPred = Pred[current]/(A * B)*( A * C * Prey[current]/(1.0f + C*Prey[current]) - 1.0f);
        
		Prey[current]=Prey[current]+(drPrey + d2Preydxy2)*dT;
        Pred[current]=Pred[current]+(drPred + d2Preddxy2)*dT;
    }
    
    barrier(CLK_GLOBAL_MEM_FENCE);
    
	// HANDLE Boundaries
	if(row==0)
    {
        Prey[row * Grid_Width+column]=Prey[(Grid_Height-2) * Grid_Width+column];
        Pred[row * Grid_Width+column]=Pred[(Grid_Height-2) * Grid_Width+column];
    }
    
	else if(row==Grid_Height-1)
    {
        Prey[row * Grid_Width + column]=Prey[1*Grid_Width+column];
        Pred[row * Grid_Width + column]=Pred[1*Grid_Width+column];
    }
	
	else if(column==0)
    {
        Prey[row * Grid_Width + column]=Prey[row * Grid_Width + Grid_Width-2];
        Pred[row * Grid_Width + column]=Pred[row * Grid_Width + Grid_Width-2];
    }
    
	else if(column==Grid_Width-1)
    {
        Prey[row * Grid_Width + column]=Prey[row * Grid_Width + 1];
        Pred[row * Grid_Width + column]=Pred[row * Grid_Width + 1];
    }
	
} // End Kernel

