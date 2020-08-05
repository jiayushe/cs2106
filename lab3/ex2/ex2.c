/*************************************
 * Lab 3 Exercise 2
 * Name: She Jiayu
 * Student No: A0188314B
 * Lab Group: 9
 *************************************/
#include <pthread.h>
#include "rw_lock.h"

void initialise(rw_lock* lock)
{
  pthread_mutex_init(&(lock->mutex), NULL);
  pthread_mutex_init(&(lock->roomEmpty), NULL);
  pthread_mutex_init(&(lock->transtile), NULL);
  lock->reader_count = 0;
  lock->writer_count = 0;
}

void writer_acquire(rw_lock* lock)
{
  pthread_mutex_lock(&(lock->transtile));
  pthread_mutex_lock(&(lock->roomEmpty));
  lock->writer_count++;
}

void writer_release(rw_lock* lock)
{
  lock->writer_count--;
  pthread_mutex_unlock(&(lock->roomEmpty));
  pthread_mutex_unlock(&(lock->transtile));
}

void reader_acquire(rw_lock* lock)
{
  pthread_mutex_lock(&(lock->transtile));
  pthread_mutex_unlock(&(lock->transtile));
  pthread_mutex_lock(&(lock->mutex));
  if (lock->reader_count == 0) {
    pthread_mutex_lock(&(lock->roomEmpty));
  }
  lock->reader_count++;
  pthread_mutex_unlock(&(lock->mutex));
}

void reader_release(rw_lock* lock)
{
  pthread_mutex_lock(&(lock->mutex));
  lock->reader_count--;
  if (lock->reader_count == 0) {
    pthread_mutex_unlock(&(lock->roomEmpty));
  }
  pthread_mutex_unlock(&(lock->mutex));
}

void cleanup(rw_lock* lock)
{
  pthread_mutex_destroy(&(lock->mutex));
  pthread_mutex_destroy(&(lock->roomEmpty));
  pthread_mutex_destroy(&(lock->transtile));
}
