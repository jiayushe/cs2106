/*************************************
 * Lab 3 Exercise 1
 * Name: She Jiayu
 * Student No: A0188314B
 * Lab Group: 9
 *************************************/


#include <pthread.h>

typedef struct {
  pthread_mutex_t mutex;
  pthread_mutex_t roomEmpty;
  int reader_count;
  int writer_count;
} rw_lock;

