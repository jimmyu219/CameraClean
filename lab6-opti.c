#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//# include "demo.h"		// Neural network 

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
	
	int projWidth = 0;
	int projHeight = 0;
	
	int roiTop = 0, roiBottom = 0;		// Row/col indexes for ROI
	int roiLeft = 0, roiRight = 0;
	
	int roiHeight = 0;
	int roiWidth = 0;
	
	int projArr[480][640] = { 0 };
	int roiArr[480][640] = { 0 };
	
	// Resize variables
	
	// Neural network variables
	
	// -----------------------------------------------------------------------------------
	// 
	// Begin system
	//
	// -----------------------------------------------------------------------------------
	printf("System restart\n");
	*oStart = 1;					// Initiate clock system
	*oClock = 0;				// Set HPS simulated clock to 0

	while(1)
	{
		// -----------------------------------------------------------------------------------
		// 
		// Reset the variables for next iteration
		//
		// -----------------------------------------------------------------------------------
		
		// -----------------------------------------------------------------------------------
		// 
		// Prompt user to begin
		//
		// -----------------------------------------------------------------------------------
		*oState = 1;				// State 1 - Ready
		printf("Press Enter to begin\n ");
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
		
		*oState = 2;				// State 2 - Reading image
		
		printf("Reading into image array\n");
		for (rows = 0; rows < 480; rows++)	// 640x480
		{	
			for(cols = 0; cols < 640; cols++)
			{
				Clock();
				Clock();
			
				while (prevAck == *iAck) {;}	// Delay loop waiting to see if HW caught up
				prevAck = *iAck;				// Get current value of counter[1]
					
				imgArr[rows][cols] = *iData;		// 0's and 1's are determined in verilog
			}
		}
		
		// Restart Clock because we're done with the SDRAM
		*oStart = 1;
		
		// Debug print loop
		/*
		printf("Printing out full image array with buffers\n");
		for (rows = 48; rows < 432; rows++)	// 640x480
		{
			for(cols = 64; cols < 576; cols++)
			{					// Get current value of counter[1]		
				if (imgArr[rows][cols])
					printf(".");
				else
					printf("0");
			}
			printf("\n");
		}
		*/
		
		// -----------------------------------------------------------------------------------
		// 
		// Detect white projector space
		//	Put a 10% buffer on each side:	cols = 64 -> 576 : imgLeftSide -> imgRightSide
		//									rows = 48 -> 432 : imgTopSide -> imgBotSide
		//
		// -----------------------------------------------------------------------------------
		*oState = 4;				// State 3 - Detect projector
		
		// Sum up rows and columns
		for (cols = colsMin; cols < colsMax; cols++)
		{
			for (rows = rowsMin; rows < rowsMax; rows++)
			{
				colsSumArr[cols] += imgArr[rows][cols];
				rowsSumArr[rows] += imgArr[rows][cols];
			}
		}
		
		// Find max value of rows sum
		for (rows = 0; rows < 480; rows++)
		{
			if (rowsSumArr[rows] > rowsSumMax)
				rowsSumMax = rowsSumArr[rows];
		}
		
		// Calculate rows level - 10% less than the max
		rowsLevel = rowsSumMax - rowsSumMax * 0.1;
		
		// Find max value of cols sum
		for (cols = 0; cols < 480; cols++)
		{
			if (colsSumArr[cols] > colsSumMax)
				colsSumMax = colsSumArr[cols];
		}
		
		// Calculate cols level - 5% less than the max
		colsLevel = colsSumMax - colsSumMax * 0.05;
		
		// Generate the array of binary values for rows and cols
		for (rows = rowsMin; rows < rowsMax; rows++)
		{
			if (rowsSumArr[rows] > rowsLevel)
				binaryRowsSumArr[rows] = 1;
			else
				binaryRowsSumArr[rows] = 0;
		}
		for (cols = colsMin; cols < colsMax; cols++)
		{
			if (colsSumArr[cols] > colsLevel)
				binaryColsSumArr[cols] = 1;
			else
				binaryColsSumArr[cols] = 0;
		}
		
		// Scan for top of white projector space
		for (cols = colsMin; cols < colsMax; cols++)
		{
			if (binaryColsSumArr[cols] == 1)
			{
				projLeft = cols;
				break;
			}
		}
		
		// Scan for bottom of white projector space
		for (cols = colsMax - 1; cols > colsMin - 1; cols--)
		{
			if (binaryColsSumArr[cols] == 1)
			{
				projRight = cols;
				break;
			}
		}
		
		// Scan for left side of white projector space
		for (rows = rowsMin; rows < rowsMax; rows++)
		{
			if (binaryRowsSumArr[rows] == 1)
			{
				projTop = rows;
				break;
			}			
		}
		
		// Scan for right side of white projector space
		for (rows = rowsMax - 1; rows > rowsMin - 1; rows--)
		{
			if (binaryRowsSumArr[rows] == 1)
			{
				projBottom = rows;
				break;
			}
		}
		
		projHeight = projBottom - projTop;
		projWidth = roiRight - projLeft;
		
		// Debugging code
		printf("projLeft: %d, projRight: %d\n", projLeft, projRight);
		printf("projTop: %d, projBottom: %d\n", projTop, projBottom);
		
		for (rows = projTop; rows < projBottom; rows++)
		{
			for (cols = projLeft; cols < projRight; cols++)
			{
				if (imgArr[rows][cols])
					printf(".");
				else
					printf("0");
			}
			printf("\n");
		}
		
		// -----------------------------------------------------------------------------------
		// 
		// Detect ROI black space
		//	Using the same methods as detecting the white projector space
		//
		// -----------------------------------------------------------------------------------
		
	} // End while(1)
	
	return 0;
}