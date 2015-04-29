#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//# include "demo.h"		// Neural network 

volatile int * oStart			= (int *) 0xFF200090;

volatile int * oClock			= (int *) 0xFF200010;		// Increments counter register in verilog
volatile int * iAck			= (int *) 0xFF200070;		// Used for counter[1] ACK

volatile int * iImgData			= (int *) 0xFF200060;
volatile int * iRowData			= (int *) 0xFF200080;
volatile int * iColData			= (int *) 0xFF200000;

volatile int * oRowAddr			= (int *) 0xFF200050;
volatile int * oColCol			= (int *) 0xFF200020;

volatile int * oState			= (int *) 0xFF200030;		// Used to show the state with LEDs
volatile int * oDigits			= (int *) 0xFF200040;		// Displays proposed digits to HEX modules

volatile int * DDR3			= (int *) 0x0010000; // Up to 0xFFF0000

volatile int * sdram		        = (int * )0xC0000000; // sdram

int main(void) {
	
	int mask = 0xFF; 
	int RECORD_TIME = 0;

	*(oStart) = 1;

	while(1){

		*oState = 1;				// State 1 - Ready
		if (RECORD_TIME)
		{
			printf("We will print out timing\n");
			printf("Press Enter to continue or enter (0) to stop timing: ");
		} else {
			printf("We will run without timing\n");
			printf("Press Enter to continue or enter (1) to timing this, and future runs: ");
		}
		
		scanf("%d", &RECORD_TIME);
		
		switch (RECORD_TIME)
		{
			case (0):	break;
			case (1):	break;
			default:	RECORD_TIME = 0;
		}
		
		*oStart = 0;				// Stop camera


		//int img [480][640];		// Array that holds image
		//int binary[480][640];	// Array that holds the binary version of the img
		
		int rows = 0;			// Indices for loops
		int cols = 0;			// Indices for loops
		

		//takes the image and convert to a binary image
		for(rows = 0 ; rows < 480 ; rows++)
		{
			for(cols = 0 ; cols < 640; cols++)
			{
				if ((*sdram & mask) > 127)
					printf(".");
				else
					printf("0");

				sdram = sdram + 1;
			}
			printf("\n");
		}
		


	}

	return 0;
}
