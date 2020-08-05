/***************************************
 * Changes made in this file will not be 
 * used for grading.
 ***************************************/
#include <stdio.h> // we haven't implemented all functionalities of stdio :)
#include <stdarg.h>
#include <string.h>
#include "my_stdio.h"

void fail_if(int cond, char *file, int line, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);

	if (cond) {
		fprintf(stdout, "%s:%d: ", file, line);
		vfprintf(stdout, fmt, ap);
		fflush(stdout);
		exit(1);
	}

	va_end(ap);
}

#define FAIL_IF(cond, ...) fail_if((cond), __FILE__, __LINE__, __VA_ARGS__)

void bad_command(int cond, char *command, const char *fmt, ...) { 
	va_list ap;

	va_start(ap, fmt);

	if (cond) {
		fprintf(stdout, "%s: ", command);
		vfprintf(stdout, fmt, ap);
		fflush(stdout);
		exit(1);
	}

	va_end(ap);
}

void initialize_buf(char *buf, int size) {
	int first = 97, last = 122, i = 0;
	while (i < size) {
		buf[i] = first + (i % (last - first + 1));
		i++;
	}
}

#define MAX_FILE_NAME 20
#define MAX_NO_FILES 10

int main(int argc, char **argv) {
	MY_FILE *open_files[MAX_NO_FILES], *f;
	char filenames[MAX_NO_FILES][MAX_FILE_NAME];
	char *command, *filename, *mode, *items, *pos, *whence, *buf;
	int ret, cur_file = 0, i;
	size_t size;
	char *line = NULL;

	while (getline(&line, &size, stdin) != -1) {
		char *newline = strchr(line, '\n');
		if (newline)
  			*newline = 0;
		command = strtok(line, "\n ");
		FAIL_IF(!command, "Command should be provided. Aborting!\n");
		filename = strtok(NULL, " ");
		bad_command(!filename, command, "filename should be provided. Aborting!\n");
		if (strcmp(command, "my_fopen") == 0) {
			mode = strtok(NULL, " ");
			bad_command(!mode, command, "mode should be provided. Aborting!\n");
			f = my_fopen(filename, mode);
			if (f)
				printf("S: File %s is now open\n", filename);
			else {
				printf("F: Could not open file %s\n", filename);
				continue;
			}
			open_files[cur_file] = f;
			strcpy(filenames[cur_file++], filename);
		}

		// The file is already open, we need to find it and have f point to it
		else {
			for (i = 0; i < MAX_NO_FILES; i++)
				if (strcmp(filename, filenames[i]) == 0)
					break;
			if (i == MAX_NO_FILES)
				printf("F: File %s is not open\n", filename);
			else {
				f = open_files[i];
				if (strcmp(command, "my_fclose") == 0) {
					ret = my_fclose(f);
					if (!ret) {
						printf("S: File %s is now closed\n", filename);
						strcpy(filenames[i], "###");
					}
					else
						printf("F: File %s could not be closed\n", filename);
				}
				else if (strcmp(command, "my_fflush") == 0) {
					ret = my_fflush(f);
					if (!ret)
						printf("S: File %s was flushed\n", filename);
					else
						printf("F: File %s could not be flushed\n", filename);
				}
				else if (strcmp(command, "my_fseek") == 0) {
					pos = strtok(NULL, " ");
					bad_command(!pos, command, "pos should be provided. Aborting!\n");
					whence = strtok(NULL, " ");
					bad_command(!whence, command, "whence should be provided. Aborting!\n");
					ret = my_fseek(f, atoi(pos), atoi(whence));
					if (ret >= 0)
						printf("S: File offset of file %s is now at position %d\n", filename, ret);
					else 
						printf("F: Could not set file offset for file %s\n", filename);
				}
				else { // either my_fread() or my_fwrite()
					items = strtok(NULL, " ");
					bad_command(!items, command, "number of items should be provided. Aborting!\n");
					if (strcmp(command, "my_fread") == 0) {
						buf = (char*)malloc(sizeof(char) * atoi(items));
						FAIL_IF(!buf, "F: Could not allocate memory for buf. Aborting!\n");
						ret = my_fread(buf, 1, atoi(items), f);
						if (ret >= 0) {
							printf("S: %d bytes were read from file %s\n", ret, filename);
							// can print the buffer to check the data is correct
						}
						else
							printf("F: Could not read %d bytes from file %s\n", atoi(items), filename);
						free(buf);
					}
					else if (strcmp(command, "my_fwrite") == 0) {
						buf = (char*)malloc(sizeof(char) * atoi(items));
						initialize_buf(buf, atoi(items));
						FAIL_IF(!buf, "F: Could not allocate memory for buf. Aborting!\n");
						ret = my_fwrite(buf, 1, atoi(items), f);
						if (ret >= 0) 
							printf("S: %d bytes were written to file %s\n", ret, filename);
						else
							printf("F: Could not write %d bytes to file %s\n", atoi(items), filename);
						free(buf);
					}
				}
			}
		}
	}
}
