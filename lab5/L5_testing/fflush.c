#include <stdio.h>

#include "my_stdio.h"
#include "util.h"
#include "hooks.h"

int num_sys_read;
int target_fd;

ssize_t hook_read(int fd, void *buf, size_t len);

struct func_hook hooks[] = {
	[0] = { .name = "read", .addr = (unsigned long)hook_read, .orig_addr = 0 },
};

ssize_t hook_read(int fd, void *buf, size_t len) {
	ssize_t (*orig_read)(int, void *, size_t);

	orig_read = (ssize_t (*)(int, void *, size_t))hooks[0].orig_addr;

	if (fd == target_fd)
		num_sys_read++;

	return orig_read(fd, buf, len);
}

int main(int argc, char **argv) {
	MY_FILE *f, *g;
	char buf[500];
	int ret, test;

	if (argc != 2) {
		printf("Please provide an argument! (1 or 2)\n");
		exit(1);
	}

	test = atoi(argv[1]);

	switch (test) {
		case 1:
			install_hooks("libmy_stdio.so", hooks, 1);
			g = my_fopen("read_test.in", "r");

			num_sys_read = 0;
			target_fd = g->fd;
			
			FAIL_IF(!g, "Could not open file\n");
			ret = my_fread(buf, 1, 500, g);
			ret = my_fflush(g);
			check(ret == 0, "return value is not correct");
			ret = my_fread(buf, 1, 500, g);
			check(num_sys_read == 2, "fflush() did not reset the buffer state");

			my_fclose(g);
			break;

		case 2:
			f = my_fopen("write_test.out", "r+");
			FAIL_IF(!f, "Could not open file\n");

			ret = my_fwrite("That", 1, 4, f);
			ret = my_fflush(f);
			check(ret == 0, "return value is not correct");
			ret = my_fread(buf, 1, 500, f);
			FAIL_IF(ret <= 0, "Could not read data\n");
			check(memcmp(buf + 4, "a file", 6) == 0, "contents are not the same");

			my_fclose(f);
	}

	return 0;
}
