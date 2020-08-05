#include <stdio.h>

#include "my_stdio.h"
#include "hooks.h"
#include "util.h"

int num_sys_write;
int target_fd;

ssize_t hook_write(int fd, void *buf, size_t len);

struct func_hook hooks[] = {
	[0] = { .name = "write", .addr = (unsigned long)hook_write, .orig_addr = 0 },
};

ssize_t hook_write(int fd, void *buf, size_t len)
{
	ssize_t (*orig_write)(int, void *, size_t);

	orig_write = (ssize_t (*)(int, void *, size_t))hooks[0].orig_addr;

	if (fd == target_fd)
		num_sys_write++;

	return orig_write(fd, buf, len);
}

int main() {
	MY_FILE *f;
	char buf[2500];
	int ret;

	install_hooks("libmy_stdio.so", hooks, 1);
	f = my_fopen("test_write.out", "w");
	FAIL_IF(!f, "Could not open file\n");

	target_fd = f->fd;
	num_sys_write = 0;
	
	// Writing 5000 characters in total => write() should be called 1 times
	ret = my_fwrite(buf, 1, 2500, f);
	FAIL_IF(ret < 0, "Could not write data\n");
	
	ret = my_fwrite(buf, 1, 2500, f);
	FAIL_IF(ret < 0, "Could not write data\n");
	
	check(num_sys_write == 1, "writes were not buffered");

	my_fclose(f);
	return 0;
}
