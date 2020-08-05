#include <stdio.h>

#include "my_stdio.h"
#include "util.h"

int main() {
	MY_FILE *f, *g;
	char buf[5000];
	int ret;

	// Test correct buffering
	f = my_fopen("read_test.in", "r");
	FAIL_IF(!f, "Could not open file\n");

	ret = my_fread(buf, 1, 100, f);
	FAIL_IF(ret < 0, "Could not read data\n");
	check(ret == 100, "return value is not correct");
	check(memcmp(buf + 34, "Alice in Wonderland", 19) == 0, "contents are not the same");
	
	ret = my_fread(buf, 1, 4000, f);
	FAIL_IF(ret < 0, "Could not read data\n");
	check(memcmp(buf + 239, "Alice in Wonderland", 19) == 0, "contents are not the same");
	check(memcmp(buf + 3955, "Alice", 5) == 0, "contents are not the same");

	my_fclose(f);
	
	// Test large read
	g = my_fopen("read_test.in", "r");
	FAIL_IF(!g, "Could not open file\n");

	ret = my_fread(buf, 1, 5000, g);
	FAIL_IF(ret < 0, "Could not read data\n");
	check(memcmp(buf + 4735, "Alice", 5) == 0, "contents are not the same");

	my_fclose(g);
	
	return 0;
}
