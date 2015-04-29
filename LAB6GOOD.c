#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
# include "demo.h"		// Neural network 

int DEBUG = 1;

volatile int * Start			= (int *) 0xFF200080;
//volatile int * Stop		= (int *) 0xFF200010;

volatile int * Clock			= (int *) 0xFF200000;

volatile int * Din			= (int *) 0xFF200050;
volatile int * Dout			= (int *) 0xFF200040;

volatile int * in1			= (int *) 0xFF200060;		// Used for coutner[1] ACK
//volatile int * in2			= (int *) 0xFF200070;
volatile int * out1			= (int *) 0xFF200020;		// Supposed to be used for test output
//volatile int * out2			= (int *) 0xFF200030;

volatile int * DDR3			= (int *) 0x0010000; // Up to 0xFFF0000


void delay(int v) {
	int c, d;
	int max;
	max = 1000 * v;
	for(c = 1; c <= max; c++)
		for(d = 1; d <= 327; d++)
		{}
}

int myRound(double x) {
	return (x - (int)x) > 0.5 ? (int)x + 1 : (int)x;
}

int main(){

	printf("Start\n");
	*(Start) = 1;
	while(1)
	{
		//move the variables back inside main for the restart purpose
		// Variables
		int img [480][640];		// Array that holds image
		//int binary[480][640];	// Array that holds the binary version of the img
		
		int rows = 0;			// Indices for loops
		int cols = 0;			// Indices for loops
		
	
		
		// Resize
		int **ROI;
		int sizeCol;
		int sizeRow;
		
		// ROI
		int rows_Sum[480] = { 0 };
		int binary_rows_Sum[480] = { 0 };
		int cols_Sum[640] = { 0 };
		int binary_cols_Sum[640] = { 0 };
		int rows_index_beg, rows_index_end;
		int cols_index_beg, cols_index_end;
		int i = 0, j = 0, k = 0;
		
		//delcare a 28x28 2d array
		int output[28][28];
		int nn[784];
		
		double Z3[10];
		int answer=0;
		int tempAns=0;

		
		printf("Press Enter to begin\n ");
		getchar();
	//delay(50);
	*(Start) = 0;
	
	int prev = *(in1);			// Get the initial value of counter[1]
	*(Clock) = 0;				// Set HPS_Clk to 0 initially
	
	// -----------------------------------------------------------------------------------
	// 
	// Ignore the first row, it is all 0's
	//
	// -----------------------------------------------------------------------------------
	for (rows = 0; rows < 640; rows++)
	{
		*Clock = 0;
		*Clock = 1;
		*Clock = 0;
		*Clock = 1;
		
		while (prev == *(in1)) {;} 		// Delay loop waiting to see if HW caught up
		prev = *(in1);					// Get current value of counter[1]
	}
	
	// -----------------------------------------------------------------------------------
	// 
	// Read SDRAM -> image[480][640] array of 1's and 0's
	//	The image is made binary in the verilog code
	//
	// -----------------------------------------------------------------------------------
	printf("Reading into image array\n");
	for (rows = 0; rows < 479; rows++)	// 640x480
	{
		// Ignore first two entries, they are 0
		*Clock = 0;
		*Clock = 1;
		*Clock = 0;
		*Clock = 1;
		*Clock = 0;
		*Clock = 1;
		*Clock = 0;
		*Clock = 1;
		
		for(cols = 0; cols < 638; cols++)
		{
			*Clock = 0;
			*Clock = 1;
			*Clock = 0;
			*Clock = 1;
		
			while (prev == *(in1)) {;}		 		// Delay loop waiting to see if HW caught up
			prev = *(in1);							// Get current value of counter[1]
				
			img[rows][cols] =  *(Din);				// Assign value to array
		}
	}
		
	printf("Image captured\n");	
	// Restart Clock because we're done with the SDRAM
	*(Start) = 1;
	
	//takes the image and convert to a binary image
	for(rows = 0 ; rows <480 ; rows++)
	{
		for(cols = 0 ; cols < 640; cols++)
		{
			if(img[rows][cols] > 127)
			{
				img[rows][cols] = 1;
			}
			else
			{
				img[rows][cols] = 0;
			}
		}
	}
	
	
	/*
	printf("Printing out image array\n");
	for (rows = 48; rows < 432; rows++)	// 640x480
	{
		for(cols = 64; cols < 576; cols++)
		{					// Get current value of counter[1]		
			//printf("%d ", img[rows][cols]);
			if (img[rows][cols])
				printf(".");
			else
				printf("0");
		}
		printf("\n");
	}
	*/
	
	// -----------------------------------------------------------------------------------
	// 
	// Detect ROI
	//	Put a 10% buffer on each side:	cols = 64 -> 576
	//									rows = 48 -> 432
	//
	// -----------------------------------------------------------------------------------
	
	// Add up columns
	for (cols = 64; cols < 576; cols++)
	{
		for (rows = 48; rows < 432; rows++)
		{
			cols_Sum[cols] += img[rows][cols];
		}
	}
	
	//hardcoding the value only work for this sample
	//base on the sum come up with binary to find the index
	for (cols = 64; cols<576; cols++)
	{
		if (cols_Sum[cols]>374) // 374 because it can be at most 512, in a perfect situation it would be 512
		{
			binary_cols_Sum[cols] = 1;
		}
		else
		{
			binary_cols_Sum[cols] = 0;
		}		
	}

	//find the index that has 0
	for (i = 64; i < 576; i++)
	{
		if (binary_cols_Sum[i] == 0)
		{
			cols_index_beg = i;
			break;
		}
	}

	//scan for the end
	for (i = 575; i > 64; i--)
	{
		if (binary_cols_Sum[i] == 0)
		{
			cols_index_end = i;
			break;
		}
	}
	
	//add up the rows
	for (rows = 48; rows < 432; rows++)
	{
		for (cols = 64; cols < 576; cols++)
		{
			rows_Sum[rows] += img[rows][cols];
		}
	}
	
	//base on the sum come up with binary to find the index
	for (rows = 48; rows<432; rows++)
	{
		if (rows_Sum[rows]>500)
		{
			binary_rows_Sum[rows] = 1;
		}
		else
		{
			binary_rows_Sum[rows] = 0;
		}

	}

	//scan for begin index
	for (j = 48; j < 432; j++)
	{
		if (binary_rows_Sum[j] == 0)
		{
			rows_index_beg = j;
			break;
		}
		
	}

	//scan for end index
	for (j = 431; j>48; j--)
	{
		if (binary_rows_Sum[j] == 0)
		{
			rows_index_end = j;
			break;
		}
	}
	
	/*
	// Print out row and col sum arrays
	printf("Columns\n");
	for (i = 64; i < 576; i++)
		printf("%d ", cols_Sum[i]);
	printf("\nRows\n");
	for (i = 48; i < 432; i++)
		printf("%d ", rows_Sum[i]);
	printf("\n");
	
	*/
	
	
	//use the index to get the region of interest
	sizeCol = cols_index_end - cols_index_beg;
	sizeRow = rows_index_end - rows_index_beg;

	
	//printf("col beg: %d, col end: %d\n", cols_index_beg, cols_index_end);
	//printf("row beg: %d, row end: %d\n", rows_index_beg, rows_index_end);
	
	
	//printf("%d\n", sizeCol);
	//printf("%d\n", sizeRow);
	
	//dynamically allocate memory for ROI
	ROI = (int **)malloc(sizeof(int *)*sizeRow);

	// for each row, malloc space for its buckets and add it to 
	// the array of arrays
	for (i = 0; i < sizeRow; i++) {
		ROI[i] = (int *)malloc(sizeof(int)*sizeCol);
	}
	
	//store the ROI into the dynamically allocated 2d array (pointer to pointer)
	for (i = cols_index_beg; i<cols_index_end; i++)
	{
		for (j = rows_index_beg; j<rows_index_end; j++)
		{
			ROI[j - rows_index_beg][i - cols_index_beg] = img[j][i];
		}
	}
	
	/*
	printf("Printing out ROI array\n");
	for (rows = 0; rows < sizeRow; rows++)	// 640x480
	{
		for(cols = 0; cols < sizeCol; cols++)
		{
			if (ROI[rows][cols])
				printf(".");
			else
				printf("0");
		}
		printf("\n");
	}
	*/
	
	// -----------------------------------------------------------------------------------
	// 
	// Segment the ROI
	//
	// -----------------------------------------------------------------------------------
	
	//find the height of the image;

	int roi_width = sizeCol;
	int roi_height = sizeRow;
	
	int digit_height = roi_height;
	int digit_width;
	
	int z;
	int x;
	int y;
	//int num_digits = round(roi_width / roi_height);
	double roundNum=0;
	roundNum=(sizeCol*1.0 / sizeRow);
	int num_digits=round(roundNum);
	/*
	if(roundNum<1.5)
	{
		num_digits = 1;
	}
	else if(roundNum<2.2)
	{
		num_digits=2;
	}
	else if(roundNum<3.2)
	{
		num_digits=3;
	}
	else if (roundNum<4.2)
	{
		num_digits = 4;
	}else if(roundNum<5.2)
	{
		num_digits = 5;
	}else if(roundNum<6.2)
	{
		num_digits=6;
	}
	*/
	//int num_digits = round(roundNum);
	printf("%d\n", roi_width);
	printf("%d\n", roi_height);
	printf("%d\n", num_digits);
	
	for (z = 0; z < num_digits; z++)
	{
		// -----------------------------------------------------------------------------------
		// 
		// Resize the segmented digit
		//
		// -----------------------------------------------------------------------------------
		/*
		//resize function
		//value to store the index
		for (i = 0; i<28; i++)
		{
			for (j = 0; j<28; j++)
			{
				x = round(i*(height - 1) / 27);
				y = round(j*(width - 1) / 27);
				//output will have the 28x28 dimensions of the image
				output[i][j] = ROI[x][y];
			}
		}
		*/
		
		// Find width of segmented digit
		digit_width = roi_width / num_digits;
		
		// Extract current digit and resize it
		for (i = 0; i<28; i++)
		{
			for (j = 0; j<28; j++)
			{
				x = round(i*(digit_height - 1) / 27);
				y = round(j*(digit_width - 1) / 27);
				
				// X -> Height, doesn't change
				// Y -> Width, the index changes as we move across the ROI
				output[i][j] = ROI[x][z*digit_width + y];
			}
		}
		
		/*
		printf("Print 28x28\n");
		for (i = 0; i<28; i++)
		{
			for (j = 0; j<28; j++)
			{
				if (output[i][j])
					printf(".");
				else
					printf("0");
			}
			printf("\n");
		}
		*/

		//convert the 28x28 into 784x1
		for (i = 0; i < 28; i++)
		{
			for (j = 0; j < 28; j++)
			{
				nn[i * 28 + j] = output[j][i]; //swap the j and i because matlab multiplys differently
			}
		}
		
		// -----------------------------------------------------------------------------------
		// 
		// Send 784x1 to Neural Network
		//
		// -----------------------------------------------------------------------------------
		
		//printf("Start Neural Network\n");
		
		double sum;
		double Z1[200];
		double Z2[200];
		
		//apply the weight 1
		//Multiplication Logic
		for (i = 0; i < 200; i++) 
		{
				sum = 0;
				for (k = 0; k < 784; k++)
				{
					sum = sum + W1[i][k] * nn[k];
				}
				Z1[i] = sum;
		
		}

		for (j = 0; j < 200; j++)
		{
			Z1[j] = Z1[j] + B1[j];
		}
	
	
		double temp;
		//
		for (i = 0; i < 200; i++)
		{
			temp = exp(-Z1[i]);
			Z1[i] = 1 / (1 + temp);
		}
	
		//apply weight 2
		//Multiplication Logic
		for (i = 0; i < 200; i++) 
		{
				sum = 0;
				for (k = 0; k < 200; k++) 
				{
					sum = sum + W2[i][k] * Z1[k];
				}
				Z2[i] = sum;
		}
	
		for (i = 0; i< 200; i++)
		{
			Z2[i] = Z2[i] + B2[i];
		}
	
		for (i = 0; i < 200; i++)
		{
			temp = exp(-Z2[i]);
			Z2[i] = 1 / (1 + temp);
		}
	
		//apply weight 3
		//Multiplication Logic
		for (i = 0; i < 10; i++)
		{
				sum = 0;
				for (k = 0; k < 200; k++) 
				{
					sum = sum + W3[i][k] * Z2[k];
				}
				Z3[i] = sum;
		}
	
		//iterate through array and find the index of the max
		int max = Z3[0];
	
		for (i = 0; i < 10; ++i)
		{
			if (Z3[i] > max)
			{
				max = Z3[i];
				tempAns = i+1; //add one to the index because the index starts with 0
			}
		}
		
		 answer = answer + pow(10.0, (num_digits-(z+1)))*(tempAns%10);
		//answer = answer % 10; //use MOD to take care of the 0 case
		printf("Digit #%d is %d\n", z+1, answer);
		//printf("Digit #%d is %d\n", z+1, answer);
	} // End segmentation for loop
	printf("The detected digits are: %d\n", answer);
	printf("\nDone\n");
	}
	
	return 0;
}