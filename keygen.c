/************************************************************
* Author: Written by Kyle Wollman (wollmank@oregonstate.edu)
* Date: 03/17/2018
* Class: CS344 400 W2018
* Description: Program 4 - keygen.c: Generates a string of
*			   random characters of length passed in from 
*			   command linefrom A-Z and space.
**************************************************************/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	srand(time(NULL));
	
	int i;
	const char *chars = "ZYXWVUTSRQPONMLK JIHGFEDCBA"; //legal characters
	
	if(argc != 2)
	{
		write(STDERR_FILENO, "USAGE: keygen keylength\n", 24);
		exit(2);
	}
	
	//convert input size to int
	int size = atoi(argv[1]);
	
	//print out random characters to stdout
	for(i = 0; i < size; i++)
	{
		printf("%c", chars[random() % 27]);
	}
	
	//finish with newline
	printf("\n");
	
	return 0;
}
