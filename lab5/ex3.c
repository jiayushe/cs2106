/*************************************
 * Lab 5 Ex3
 * Name: She Jiayu
 * Student No: A0188314B
 * Lab Group: 9
 *************************************
 Warning: Make sure your code works on
 lab machine (Linux on x86)
 *************************************/

#include "my_stdio.h"

size_t my_fwrite(const void *ptr, size_t size, size_t nmemb, MY_FILE *stream) {
	if (!stream->write_mode || stream->buffer_state == READ_STATE) {
		// Error
		return -1;
	}
	size_t request = size * nmemb;
	size_t success = 0;
	size_t cur = MIN(BUFFER_SIZE - stream->buffer_size, request);
	memcpy(stream->buffer + stream->buffer_size, ptr, cur);
	success += cur;
	request -= cur;
	stream->buffer_size += cur;
	while (request > 0) {
		if (stream->append_mode) {
			// Append mode
			lseek(stream->fd, 0, SEEK_END);
		}
		ssize_t write_size = write(stream->fd, stream->buffer, BUFFER_SIZE);
		if (write_size == -1) {
			// Error
			return MY_EOF;
		}
		stream->buffer_size = 0;
		size_t cur = MIN(BUFFER_SIZE, request);
		memcpy(stream->buffer, ptr, cur);
		success += cur;
		request -= cur;
		stream->buffer_size += cur;
	}
	// Set state to 'write'
	stream->buffer_state = WRITE_STATE;
	return size ? success / size : 0;
}
