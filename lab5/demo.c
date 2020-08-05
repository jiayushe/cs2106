/*************************************
 * Lab 5 Demo
 * Name: She Jiayu
 * Student No: A0188314B
 * Lab Group: 9
 *************************************
 Warning: Make sure your code works on
 lab machine (Linux on x86)
 *************************************/
#include "my_stdio.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void freadFromFile (char who[50], MY_FILE *file);

int main(int argc, char **argv) {
    char* fileName = NULL;
    int nChild = 0;
    int i =0, childPID = 0;
    MY_FILE *file;
    char who[50];
    ssize_t readBytes = 1;
	char charBuf = 0;

    if (argc < 3) {
        printf ("Usage: %s <filename> <number_of_processes>\n", argv[0]);
        return 1;
    }
    
    fileName = argv[1];
    nChild = atoi(argv[2]);

    file = my_fopen( fileName, "r" );

    sprintf(who, "Parent [%d]", getpid());
    
    charBuf = 0;    
    // readBytes = my_fread( (void*) &charBuf, 1, 1, file);

	if( readBytes != 1 ) {
		return 1;
	}

    
    printf("MY_FILE %s: %c\n", who, charBuf);
    
    for (i = 0; i < nChild; i++) {
        childPID = fork();
        if (childPID ==0) {
            sprintf(who, "Child %d [%d]", i+1, getpid());
            break;
        }
    }

    freadFromFile(who, file);
    my_fclose (file);

    
}   
void freadFromFile (char who[50], MY_FILE *file) {
	ssize_t readBytes = 1;
	char charBuf = 0;

	while (readBytes > 0) {
		usleep (1000);
		charBuf = 0;
		readBytes = my_fread( (void*) &charBuf, 1, 1, file);

		if( readBytes != 1 ) {
			return;
		}
        printf("MY_FILE %s: %c\n", who, charBuf);
	    
	}
}

