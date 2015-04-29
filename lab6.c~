#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//# include "demo.h"		// Neural network 

volatile int * oStart			= (int *) 0xFF2000E0;

volatile int * iAck			= (int *) 0xFF2000C0;		// Used for counter[1] ACK

volatile int * iImgData			= (int *) 0xFF2000B0;
volatile int * iRowData			= (int *) 0xFF2000D0;
volatile int * iColData			= (int *) 0xFF200050;

volatile int * oRowAddr			= (int *) 0xFF200050;
volatile int * oColAddr			= (int *) 0xFF200020;

volatile int * oState			= (int *) 0xFF200080;		// Used to show the state with LEDs
volatile int * oDigits			= (int *) 0xFF200090;		// Displays proposed digits to HEX modules

volatile int * RowAddr			= (int *) 0xFF200030;
volatile int * ColAddr			= (int *) 0xFF200020;

volatile int * HPS_CLK			= (int *) 0xFF200060;
volatile int * ReadWrite		= (int *) 0xFF200040;

volatile int * HPS_toMem		= (int *) 0xFF2000A0;
volatile int * HPS_fromMem		= (int *) 0xFF200070;

volatile int * Row_Read		= (int *) 0xFF200010;
volatile int * Col_Read		= (int *) 0xFF200000;


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


int myPow(int n)
{
	int x[9] = {1, 10, 100, 1000, 10000, 100000, 10000000, 100000000, 1000000000};
	return x[n];
}

int myMod(int n)
{
	int x[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
	return x[n];
}

double mySigmoid(double x)
{
	if (x > 5) 
		return 0;
	else if (x < -5) 
		return 1;
	else 
		return 1/(1 + exp(x));
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
	int segmentIntensity = 0;			// Accumulator to check if the segment isn't all white pixels
	int digitArr[784] = { 0 };
	
	// Neural network variables
	double sum;
	double Z1[200];
	double Z2[200];
	int max = 0;
	int pos = 0;
	
	int answer = 0;
	
	// TESTING VARIABLE
	int temp;
	
	// Timing variables
	int RECORD_TIME = 0;				// Variable that decides if we print (1) or not (0)
	
	// -----------------------------------------------------------------------------------
	// 
	// Begin system
	//
	// -----------------------------------------------------------------------------------
	printf("System restart\n");
	*oStart = 1;					// Initiate clock system
	*HPS_CLK = 0;				// Set HPS simulated clock to 0
	*oDigits = 0;

	
	*ReadWrite = 0;
	*HPS_toMem = 2;
	*Row_Read = 1;
	*Col_Read = 1;
	*RowAddr = 8;
	*ColAddr = 7;


	*HPS_CLK = 1;
	*HPS_CLK = 0;

	*ReadWrite = 1;
	*Row_Read = 1;
	*Col_Read = 1;
	*RowAddr = 8;
	*ColAddr = 7;
	printf("%d \n", *HPS_fromMem);

	*HPS_CLK = 1;
	printf("Done\n");
	return 0;
}
