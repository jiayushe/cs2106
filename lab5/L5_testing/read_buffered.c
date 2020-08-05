#include <stdio.h>

#include "my_stdio.h"
#include "hooks.h"
#include "util.h"

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

int main() {
	MY_FILE *f;
	char buf[100];
	int ret;

	install_hooks("libmy_stdio.so", hooks, 1);

	f = my_fopen("read_test.in", "r");
	FAIL_IF(!f, "Could not open file\n");

	target_fd = f->fd;
	num_sys_read = 0;

	ret = my_fread(buf, 1, 100, f);
	FAIL_IF(ret < 0, "Could not read data\n");
	ret = my_fread(buf, 1, 100, f);
	FAIL_IF(ret < 0, "Could not read data\n");
	
	check(num_sys_read == 1, "reads were not buffered");
	my_fclose(f);

	return 0;
}
