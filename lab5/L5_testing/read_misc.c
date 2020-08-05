#include <stdio.h>

#include "my_stdio.h"
#include "util.h"

int main(int argc, char **argv) {
	MY_FILE *f, *g, *h, *i;
	char buf[1000], *buf2;
	int ret, test;

	if (argc != 2) {
		printf("Please specify an argument! (1, 2, or 3)\n");
		exit(1);
	}
	test = atoi(argv[1]);

	switch (test) {
		case 1:
			// Test illegal operations
			f = my_fopen("impossible.in", "w");
			FAIL_IF(!f, "Could not open file\n");
			ret = my_fread(buf, 1, 100, f);
			if (ret > 0)
				printf("FAIL, should not be able to read from output stream\n");
			else {
				g = my_fopen("impossible2.in", "a");
				FAIL_IF(!g, "Could not open file\n");

				ret = my_fread(buf, 1, 100, g);
				check(ret <= 0, "should not be able to read from output stream");
				my_fclose(g);
			}
			my_fclose(f);
			break;

		case 2:
			// Test correctness for different items sizes
			h = my_fopen("read_test.in", "r");
			FAIL_IF(!h, "Could not open file\n");

			ret = my_fread(buf, 8, 8, h);
			check(ret == 8, "return value is not correct");
			check(memcmp(buf + 34, "Alice in Wonderland", 19) == 0, "contents are not the same");

			ret = my_fread(buf, 15, 15, h);
			check(memcmp(buf + 180, "Project Gutenberg License", 25) == 0, "contents are not the same");
			my_fclose(h);
			break;

		case 3:
			// Test correctness on small files
			i = my_fopen("read_test2.in", "r");
			FAIL_IF(!i, "Could not open file\n");

			ret = my_fread(buf, 1, 100, i);
			check(ret == 26, "return value is not correct");

			buf2 = calloc(10, sizeof(char));
			FAIL_IF(!buf2, "Could not allocate memory\n");
			ret = my_fread(buf, 1, 10, i);
			check(ret <= 0 && buf2[0] == '\0', "EOF was ignored");
			my_fclose(i);
			break;
	}

	return 0;
}
