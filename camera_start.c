#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


volatile int * ptr      = (int * )0xFF200000;		// SW

int main(){
	
	int display = 0;
	int buff=0;
	
	while(1)
	{
		printf("Do you want to start? y/n ");
		scanf("%d", &buff);
		
		if (buff == 1)
			*(ptr) = 1;
		else	
			*(ptr) = 0;
		printf("Hi\n");
	}

	return 0;
}