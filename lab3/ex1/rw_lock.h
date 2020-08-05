/*************************************
 * Lab 3 Exercise 1
 *************************************
You may change this file during your own testing, 
but note that they will be replaced with the original 
files when we test your assignments.
 *************************************/



#include <pthread.h> // To import the pthread_mutex_t type.
#include "rw_lock_struct.h"


void initialise(rw_lock* lock);
void writer_acquire(rw_lock* lock);
void writer_release(rw_lock* lock);
void reader_acquire(rw_lock* lock);
void reader_release(rw_lock* lock);
void cleanup(rw_lock* lock);

