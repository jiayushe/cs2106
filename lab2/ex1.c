/*************************************
 * Lab 2 Exercise 1
 * Name: She Jiayu
 * Student No: A0188314B
 * Lab Group: 9
 *************************************
 Warning: Make sure your code works on
 lab machine (Linux on x86)
 *************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     //for fork(), wait()
#include <string.h>
#include <math.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void readFromFile (char who[50], int fd);
void readFromFile2 (char who[50], FILE* file);

int main(int argc, char **argv) {
    char* fileName = NULL;
    int nChild = 0;
    int fd = 0;
    char who[50];
    
    if (argc < 3)
    {
        printf ("Usage: %s <filename> <number_of_processes>\n", argv[0]);
        return 1;
    }
    
    fileName = argv[1];
    nChild = atoi(argv[2]);

    fd = open( fileName, O_RDONLY );
    //TODO check the error code
    if (fd == -1) {
        printf("Error openning file%s.\n", fileName);
        return -1;
    }
    //TODO add your code
    char charBuf = 0;
    char strBuf[128];
    read(fd, &charBuf, 1);
    sprintf(who, "Parent [%d]", getpid());
    sprintf(strBuf, "%s: %c\n", who, charBuf);
    write(STDOUT_FILENO, strBuf, strlen(strBuf));

    int pid[nChild];
    for (int i = 0; i < nChild; i++) {
        pid[i] = fork();
        if (pid[i] == 0) {
            char curWho[50];
            sprintf(curWho, "Child %d[%d]", i + 1, getpid());
            readFromFile(curWho, fd);
            close(fd);
            exit(0);
        }
    }

    readFromFile(who, fd);
    close(fd);

    //TODO add your code
    for (int i = 0; i < nChild; i++) {
        int curPid = waitpid(pid[i], NULL, 0);
        printf("Parent: Child %d[%d] done.\n", i + 1, curPid);
    }

    FILE* file;
    file = fopen(fileName, "r");
    if (file == NULL) {
        printf("Error opening file%s.\n", fileName);
        return -1;
    }
    fread(&charBuf, sizeof(char), 1, file);
    sprintf(who, "Parent [%d]", getpid());
    sprintf(strBuf, "%s: %c\n", who, charBuf);
    write(STDOUT_FILENO, strBuf, strlen(strBuf));
    
    for (int i = 0; i < nChild; i++) {
        pid[i] = fork();
        if (pid[i] == 0) {
            char curWho[50];
            sprintf(curWho, "Child %d[%d]", i + 1, getpid());
            readFromFile2(curWho, file);
            fclose(file);
            exit(0);
        }
    }

    readFromFile2(who, file);
    fclose(file);
    
    for (int i = 0; i < nChild; i++) {
        int curPid = waitpid(pid[i], NULL, 0);
        printf("Parent: Child %d[%d] done.\n", i + 1, curPid);
    }

    printf("Parent: Exiting.\n");
} 

void readFromFile (char who[50], int fd) 
{
	ssize_t readBytes = 1;
	char charBuf = 0;
    char strBuf[128];

	while (readBytes > 0) {
		usleep (1000);
		charBuf = 0;
		readBytes = read( fd, &charBuf, 1 );

		if( readBytes != 1 ) 
        {
			if( readBytes == 0 ) 
            {
				// printf( "EOF\n");
				return;
			}
		}
        sprintf(strBuf, "%s: %c\n", who, charBuf);
	    write (STDOUT_FILENO, strBuf, strlen(strBuf));
		//TODO add your code
	}
}

void readFromFile2 (char who[50], FILE* file) 
{
	ssize_t readBytes = 1;
	char charBuf = 0;
    char strBuf[128];

	while (readBytes > 0) {
		usleep (1000);
		charBuf = 0;
		readBytes = fread(&charBuf, sizeof(char), 1, file);

		if( readBytes != 1 ) 
        {
			if( readBytes == 0 ) 
            {
				// printf( "EOF\n");
				return;
			}
		}
        sprintf(strBuf, "%s: %c\n", who, charBuf);
	    write (STDOUT_FILENO, strBuf, strlen(strBuf));
		//TODO add your code
	}
}
