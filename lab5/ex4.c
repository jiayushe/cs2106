/*************************************
 * Lab 5 Ex4
 * Name: She Jiayu
 * Student No: A0188314B
 * Lab Group: 9
 *************************************
 Warning: Make sure your code works on
 lab machine (Linux on x86)
 *************************************/

#include "my_stdio.h"

int my_fflush(MY_FILE *stream) {
	if (stream->buffer_state == READ_STATE) {
		lseek(stream->fd, -stream->buffer_size, SEEK_CUR);
	} else if (stream->buffer_state == WRITE_STATE) {
		if (stream->append_mode) {
			// Append mode
			lseek(stream->fd, 0, SEEK_END);
		}
		ssize_t write_size = write(stream->fd, stream->buffer, stream->buffer_size);
		if (write_size == -1) {
			// Error
			return MY_EOF;
		}
	}
	// Reset buffer
	stream->buffer_head = 0;
	stream->buffer_size = 0;
	stream->buffer_state = DEFAULT_STATE;
	return 0;
}

int my_fseek(MY_FILE *stream, long offset, int whence) {
	my_fflush(stream);
	return lseek(stream->fd, offset, whence);
}