#include <stdio.h>

#include "my_stdio.h"
#include "util.h"

int main() {
	MY_FILE *f, *g, *i;
	FILE *h;
	char buf[5000];
	int ret;

	// Test large write
	init_write_buffer(buf, 5000);
	g = my_fopen("test_write.out", "w");
	FAIL_IF(!g, "Could not open file\n");
	ret = my_fwrite(buf, 1, 5000, g);
	check(ret == 5000, "return value is not correct");
	my_fclose(g);

	h = fopen("test_write.out", "r");
	FAIL_IF(!h, "Could not open file\n");
	ret = fread(buf, 1, 4096, h);
	check(ret == 4096, "not enough data in the file");
	check(memcmp(buf + 1001, "It is possible to commit", 24) == 0, "data is not correct");
	fclose(h);


	// Test variable size and # elements
	init_write_buffer(buf, 5000);
	i = my_fopen("test_write.out", "w");
	FAIL_IF(!i, "Could not open file\n");
	ret = my_fwrite(buf, 10, 10, i);
	check(ret == 10, "return value is not correct");
	ret = my_fwrite(buf, 5, 5, i);
	ret = my_fwrite(buf, 1, 4096, i);
	my_fclose(g);

	h = fopen("test_write.out", "r");
	FAIL_IF(!h, "Could not open file\n");
	ret = fread(buf, 1, 125, h);
	check(ret == 125, "not enough data in the file");
	check(memcmp(buf + 99, "sI", 2) == 0, "data is not correct");
	fclose(h);

	f = my_fopen("read_test.in", "r");
	FAIL_IF(!f, "Could not open file\n");
	ret = my_fwrite(buf, 1, 100, f);
	check(ret <= 0, "should not be able to write to input stream");
	my_fclose(f);

	return 0;
}
