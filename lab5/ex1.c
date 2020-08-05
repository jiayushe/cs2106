/*************************************
 * Lab 5 Ex1
 * Name: She Jiayu
 * Student No: A0188314B
 * Lab Group: 9
 *************************************
 Warning: Make sure your code works on
 lab machine (Linux on x86)
 *************************************/

#include "my_stdio.h"

static int parse_mode(const char *mode, unsigned int *open_flags) {
	int basic_open_flags;
	int extra_open_flags = 0;
	*open_flags = 0;

	switch (mode[0]) {
	case 'r':
		basic_open_flags = O_RDONLY;
		break;
	case 'w':
		basic_open_flags = O_WRONLY;
		extra_open_flags = (O_CREAT|O_TRUNC);
		break;
	case 'a':
		basic_open_flags = O_WRONLY;
		extra_open_flags = O_CREAT;
		break;
	default:
		// didn't get any of r|w|a
		return 0;
	}

	if (mode[1] == '+') {
		basic_open_flags = O_RDWR;
	}

	*open_flags = basic_open_flags|extra_open_flags;

	return 1;
}

MY_FILE *my_fopen(const char *pathname, const char *mode) {
	MY_FILE *f;
	int ret;
	unsigned int open_flags;
	int fd;

	ret = parse_mode(mode, &open_flags);
	if (!ret) {
		return NULL;
	}

	fd = open(pathname, open_flags, 0644);
	if (fd < 0) {
		return NULL;
	}

	f = malloc(sizeof(*f));
	if (!f) {
		return NULL;
	}

	f->fd = fd;
	// TODO: Initialize the other members of your structure
	f->buffer = malloc(BUFFER_SIZE);
	f->buffer_head = 0;
	f->buffer_size = 0;
	f->buffer_state = DEFAULT_STATE;
	f->append_mode = (mode[0] == 'a');
	f->read_mode = (mode[0] == 'r') || (mode[1] == '+');
	f->write_mode = (mode[0] == 'w') || (mode[0] == 'a') || (mode[1] == '+');

	return f;
}

void free_file(MY_FILE *f) {
	// TODO: free() members of your structure as needed
	free(f->buffer);
	free(f);
}

int my_fclose(MY_FILE *f) {
	int ret1;
	int ret2;
	int fd = f->fd;

	ret1 = my_fflush(f);
	free_file(f);

	ret2 = close(fd);
	
	if (ret1 || (ret2 < 0))
		return MY_EOF;

	return 0;
}
