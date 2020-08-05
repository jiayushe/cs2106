#include <stdio.h>

#include "my_stdio.h"
#include "util.h"

int main(int argc, char **argv) {
	MY_FILE *f, *g;
	char buf[500], *buf2;
	int ret, test;

	if (argc != 2) {
		printf("Please provide an argument! (1 or 2)\n");
		exit(1);
	}

	test = atoi(argv[1]);

	switch (test) {
		case 1:
			f = my_fopen("read_test.in", "r");
			FAIL_IF(!f, "Could not open file\n");

			ret = my_fseek(f, 50, SEEK_SET);
			check(ret == 50, "return value is not correct");

			ret = my_fseek(f, -21000, SEEK_END);
			check(ret == 53703, "return value is not correct");

			ret = my_fread(buf, 1, 500, f);
			FAIL_IF(ret <= 0, "Could not read data\n");
			check(memcmp(buf + 48, "said the King", 13) == 0, "contents are not the same");

			ret = my_fseek(f, 15, SEEK_CUR);
			check(ret == 54218, "return value is not correct");
			
			my_fclose(f);
			break;

		case 2:
			g = my_fopen("read_test2.in", "r+");
			FAIL_IF(!g, "Could not open file\n");
			
			ret = my_fread(buf, 1, 15, g);
			FAIL_IF(ret <= 0, "Could not read data\n");
			ret = my_fseek(g, 0, SEEK_CUR);
			ret = my_fwrite("large", 1, 5, g);
			ret = my_fseek(g, 0, SEEK_SET);
			buf2 = calloc(27, sizeof(char));
			FAIL_IF(!buf2, "Could not allocate memory\n");
			ret = my_fread(buf2, 1, 26, g);
			check(memcmp(buf2, "This is a very large file!", 26) == 0, "contents are not the same");

			my_fclose(g);
	}

	return 0;
}
