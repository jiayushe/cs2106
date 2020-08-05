/*************************************
 * Lab 5 Ex2
 * Name: She Jiayu
 * Student No: A0188314B
 * Lab Group: 9
 *************************************
 Warning: Make sure your code works on
 lab machine (Linux on x86)
 *************************************/

#include "my_stdio.h"

size_t my_fread(void *ptr, size_t size, size_t nmemb, MY_FILE *stream) {
	if (!stream->read_mode || stream->buffer_state == WRITE_STATE) {
		// Error
		return -1;
	}
	size_t request = size * nmemb;
	size_t success = 0;
	size_t cur = MIN(stream->buffer_size, request);
	memcpy(ptr, stream->buffer + stream->buffer_head, cur);
	success += cur;
	request -= cur;
	stream->buffer_size -= cur;
	// Reset buffer_head to 0 if buffer is empty
	stream->buffer_head = stream->buffer_size ? stream->buffer_head + cur : 0;
	while (request > 0) {
		ssize_t read_size = read(stream->fd, stream->buffer, BUFFER_SIZE);
		if (read_size == -1) {
			// Error
			return MY_EOF;
		} else if (read_size == 0) {
			// EOF
			break;
		}
		stream->buffer_size = read_size;
		size_t cur = MIN(stream->buffer_size, request);
		memcpy(ptr + success, stream->buffer + stream->buffer_head, cur);
		success += cur;
		request -= cur;
		stream->buffer_size -= cur;
		// Reset buffer_head to 0 if buffer is empty
		stream->buffer_head = stream->buffer_size ? stream->buffer_head + cur : 0;
	}
	// Set state to 'read'
	stream->buffer_state = READ_STATE;
	return size ? success / size : 0;
}
