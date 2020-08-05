#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

void fail_if(int cond, char *file, int line, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	if (cond) {
		fprintf(stderr, "%s:%d: ", file, line);
		vfprintf(stderr, fmt, ap);
		fflush(stderr);
		exit(1);
	}

	va_end(ap);
}

void check(int cond, char *issue) {
	if (cond)
		printf("PASS \n");
	else
		printf("FAIL: %s \n", issue);
	fflush(stdout);
}

void init_write_buffer(char *buf, int size) {
	int i = 0;
	char string[] = "It is possible to commit no mistakes and still lose. That is not a weakness, that is life. ";
	while (i < size - 1) {
		buf[i] = string[i % (int)strlen(string)];
		i++;
	}
	buf[i] = '\0'; 
}
