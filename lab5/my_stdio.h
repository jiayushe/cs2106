/*************************************
 * Lab 5 my_stdio.h
 * Name: She Jiayu
 * Student No: A0188314B
 * Lab Group: 9
 *************************************
 Warning: Make sure your code works on
 lab machine (Linux on x86)
 *************************************/

/*
 * CS2106 my_stdio Library
 */

#ifndef MY_STDIO_H
#define MY_STDIO_H

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#define MY_EOF (-1)
#define BUFFER_SIZE 4096
#define DEFAULT_STATE 0
#define READ_STATE (-1)
#define WRITE_STATE 1
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

typedef struct {
	int fd;
	// TODO: Define the members of your structure
	void* buffer;
	int buffer_head;
	int buffer_size;
	int buffer_state;
	int append_mode;
	int read_mode;
	int write_mode;
} MY_FILE; 

MY_FILE *my_fopen(const char *pathname, const char *mode);
int my_fclose(MY_FILE *stream);

size_t my_fread(void *ptr, size_t size, size_t nmemb, MY_FILE *stream);
size_t my_fwrite(const void *ptr, size_t size, size_t nmemb, MY_FILE *stream);
int my_fflush(MY_FILE *stream);
int my_fseek(MY_FILE *stream, long offset, int whence);

#endif /* MY_STDIO_H */
