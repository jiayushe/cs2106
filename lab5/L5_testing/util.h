#ifndef TEST_UTIL_H
#define TEST_UTIL_H

void init_write_buffer(char *buf, int size);

void fail_if(int cond, char *file, int line, const char *fmt, ...);

void check(int cond, char *issue);

#define FAIL_IF(cond, ...) fail_if((cond), __FILE__, __LINE__, __VA_ARGS__)

#endif // TEST_UTIL_H

