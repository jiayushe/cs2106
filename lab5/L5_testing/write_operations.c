#include <stdio.h>

#include "my_stdio.h"
#include "util.h"

int main(int argc, char **argv) {
	MY_FILE *f, *h;
	FILE *g;
	char buf[2500], buf2[8192];
	int ret;

	init_write_buffer(buf, 2500);

	f = my_fopen("test_write.out", "w");
	FAIL_IF(!f, "Could not open file\n");

	ret = my_fwrite(buf, 1, 2500, f);
	FAIL_IF(ret < 0, "Could not write data\n");
	check(ret == 2500, "return value is not correct");

	ret = my_fwrite(buf, 1, 2500, f);
	FAIL_IF(ret < 0, "Could not write data\n");

	ret = my_fwrite(buf, 1, 2500, f);
	FAIL_IF(ret < 0, "Could not write data\n");

	ret = my_fwrite(buf, 1, 2500, f);
	FAIL_IF(ret < 0, "Could not write data\n");

	my_fclose(f);

	// There should be at least 8192 characters in the file, even without my_fflush()
	g = fopen("test_write.out", "r");
	FAIL_IF(!g, "Could not open file\n");
	ret = fread(buf2, 1, 8192, g);
	check(ret == 8192, "not enough data in the file");
	check(memcmp(buf2 + 4100, "That is not a weakness, that is life.", 37) == 0, "data is not correct");
	fclose(g);

	h = my_fopen("test_append.out", "a");
	FAIL_IF(!h, "Could not open file\n");

	ret = my_fwrite(" our life is what our thoughts make it.\n", 1, 40, h);
	FAIL_IF(ret < 0, "Could not write data\n");
	// Ensure that data is committed to the disk
	ret = my_fwrite(buf, 1, 2500, h);
	FAIL_IF(ret < 0, "Could not write data\n");
	ret = my_fwrite(buf, 1, 2500, h);
	FAIL_IF(ret < 0, "Could not write data\n");

	my_fclose(h);

	g = fopen("test_append.out", "r");
	FAIL_IF(!g, "Could not open file\n");
	ret = fread(buf2, 1, 64, g);
	check(ret == 64, "not enough data in the file");
	check(memcmp(buf2 + 24, "our life is what our thoughts make it.\n", 39) == 0, "data is not correct");
	fclose(g);

	return 0;
}
