#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//# include "demo.h"		// Neural network 

int RECORD_TIME = 1;

volatile int * oStart			= (int *) 0xFF200080;
//volatile int * Stop			= (int *) 0xFF200010;

volatile int * oClock			= (int *) 0xFF200000;

volatile int * iData			= (int *) 0xFF200050;
//volatile int * oData			= (int *) 0xFF200040;

volatile int * iAck				= (int *) 0xFF200060;		// Used for coutner[1] ACK
//volatile int * in2			= (int *) 0xFF200070;
volatile int * oState			= (int *) 0xFF200020;		// Used to show the state with LEDs
volatile int * oDigits			= (int *) 0xFF200030;		// Displays proposed digits to HEX modules

//volatile int * DDR3			= (int *) 0x0010000; 	// Up to 0xFFF0000


void delay(int v)
{
	int c, d;
	int max;
	max = 1000 * v;
	for(c = 1; c <= max; c++)
		for(d = 1; d <= 327; d++)
		{}
}

int myRound(double x)
{
	return (x - (int)x) > 0.5 ? (int)x + 1 : (int)x;
}

void Clock(void)
{
	*oClock = 0;
	*oClock = 1;
	*oClock = 0;
	*oClock = 1;
}

static inline unsigned int getCycles ()
{
  unsigned int cycleCount;
  // Read CCNT register
  asm volatile ("MRC p15, 0, %0, c9, c13, 0\t\n": "=r"(cycleCount));  
  return cycleCount;
}
static inline void initCounters ()
{
  // Enable user access to performance counter
  asm volatile ("MCR p15, 0, %0, C9, C14, 0\t\n" :: "r"(1));
  // Reset all counters to zero
  int MCRP15ResetAll = 23; 
  asm volatile ("MCR p15, 0, %0, c9, c12, 0\t\n" :: "r"(MCRP15ResetAll));  
  // Enable all counters:  
  asm volatile ("MCR p15, 0, %0, c9, c12, 1\t\n" :: "r"(0x8000000f));  
  // Disable counter interrupts
  asm volatile ("MCR p15, 0, %0, C9, C14, 2\t\n" :: "r"(0x8000000f));
  // Clear overflows:
  asm volatile ("MCR p15, 0, %0, c9, c12, 3\t\n" :: "r"(0x8000000f));
}

int main(void){
	
	// -----------------------------------------------------------------------------------
	// 
	// Variables
	//
	// -----------------------------------------------------------------------------------
	
	// Image variables
	int imgArr[480][640];				// Array that holds image
	int prevAck;					// Hold the previous ACK value, we check if that changes
	
	// We use a buffer of 10% on each side
	int colsMin		= 64; 		// 640*10%
	int colsMax		= 576; 		// 640 - 640*10%
	int rowsMin		= 48;		// 480*10%
	int rowsMax		= 432; 		// 480 - 480*10%
	
	// Loop indexes
	int rows, cols;					
	int i, j, k, x, y;

	// ROI variables
	int rowsSumArr[480] = { 0 };	// Holds summation of all rows
	int colsSumArr[640] = { 0 };	// Holds summation of all cols
	
	int rowsSumMax = 0;			// These holds values for the light level
	int colsSumMax = 0;
	int rowsLevel = 0;
	int colsLevel = 0;
	
	int binaryRowsSumArr[480] = { 0 };	// We transform these values into binary values
	int binaryColsSumArr[640] = { 0 };	//    according to the calculated level
	
	int projTop = 0, projBottom = 0;	// Row/col indexes for projector space
	int projLeft = 0, projRight = 0;
	
	int roiTop = 0, roiBottom = 0;		// Row/col indexes for ROI
	int roiLeft = 0, roiRight = 0;
	
	// Segmentation and Resize variables
	int numDigits = 0;
	int currentDigit = 0;
	int digitWidth = 0;
	int digitHeight = 0;
	int digitArr[784] = { 0 };
	
	// Neural network variables
	double sum;
	double Z1[200];
	double Z2[200];
	int max = 0;
	int pos = 0;
	
	int answer = 0;
	
	// -----------------------------------------------------------------------------------
	// 
	// Begin system
	//
	// -----------------------------------------------------------------------------------
	printf("System restart\n");
	*oStart = 1;					// Initiate clock system
	*oClock = 0;				// Set HPS simulated clock to 0
	*oDigits = 0;
	
	if (RECORD_TIME)
	{
		initCounters(); 
		unsigned int time;
	}

	while(1)
	{
		// -----------------------------------------------------------------------------------
		// 
		// Reset the variables for next iteration
		//
		// -----------------------------------------------------------------------------------
		for (i = 0; i < sizeof(colsSumArr); i++) colsSumArr [i] = 0;
		for (i = 0; i < sizeof(rowsSumArr); i++) rowsSumArr[i] = 0;
		
		max = -100000000;
		answer = 0;
		
		// -----------------------------------------------------------------------------------
		// 
		// Prompt user to begin
		//
		// -----------------------------------------------------------------------------------
		*oState = 1;				// State 1 - Ready
		printf("Press Enter to begin");
		getchar();
		*oStart = 0;				// Stop camera
		delay(2);					// Delay to allow verilog to settle
		prevAck = *iAck;			// Get initial ACK value
		
		// -----------------------------------------------------------------------------------
		// 
		// Read SDRAM -> imgArr[480][640] array of 1's and 0's
		//	The image is made either 255 or 0 in the verilog code
		//  Convert to 0's and 1's in C
		//
		// -----------------------------------------------------------------------------------
		
		*oState = 3;				// State 2 - Reading image
		
		if (RECORD_TIME) time = getCycles();
			
		for (rows = 0; rows < 480; rows++)	// 640x480
		{	
			for(cols = 0; cols < 640; cols++)
			{
				Clock();
			
				while (prevAck == *iAck) {;}	// Delay loop waiting to see if HW caught up
				prevAck = *iAck;				// Get current value of counter[1]
					
				imgArr[rows][cols] = *iData;		// 0's and 1's are determined in verilog
			}
		}
		
		if (RECORD_TIME) printf("Time to extract image: %d cycles\n", getCycles() - time);
		
		// Restart Clock because we're done with the SDRAM
		*oStart = 1;
		
		// -----------------------------------------------------------------------------------
		// 
		// Try FAST Corner detection algorithm
		//	For each pixel, poll the pixels within a circle of radius 4
		//
		// -----------------------------------------------------------------------------------
		for (rows = rowsMin; rows < rowsMax; rows++)	// 640x480
		{
			for(cols = colsMin; cols < colsMax; cols++)
			{		
				// Top left black corner detection
				if (
					imgArr[rows - 1][cols + 3] == 1 &&
					imgArr[rows + 0][cols + 3] == 0 &&
					imgArr[rows + 1][cols + 3] == 0 &&
					
					imgArr[rows + 2][cols + 2] == 0 &&
					
					imgArr[rows + 3][cols + 1] == 0 &&
					imgArr[rows + 3][cols + 0] == 0 &&
					imgArr[rows + 3][cols - 1] == 1 &&
					
					imgArr[rows + 2][cols - 2] == 1 &&
					
					imgArr[rows + 1][cols - 3] == 1 &&
					imgArr[rows + 0][cols - 3] == 1 &&
					imgArr[rows - 1][cols - 3] == 1 &&
					
					imgArr[rows - 2][cols - 2] == 1 &&
					
					imgArr[rows - 3][cols - 1] == 1 &&
					imgArr[rows - 3][cols + 0] == 1 &&
					imgArr[rows - 3][cols + 1] == 1 &&
					
					imgArr[rows - 2][cols + 2] == 1 
				)
				{
					printf("Top left corner at row: %d col: %d\n", rows, cols);
				}
			}
			printf("\n");
		}
		
		/*
		// Debug print loop
		printf("Printing out full image array with buffers\n");
		for (rows = rowsMin; rows < rowsMax; rows++)	// 640x480
		{
			for(cols = colsMin; cols < colsMax; cols++)
			{					// Get current value of counter[1]		
				if (imgArr[rows][cols])
					printf(" ");
				else
					printf("0");
			}
			printf("\n");
		}
		*/
		
	} // while(1)
} // main()