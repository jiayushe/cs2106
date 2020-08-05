/*************************************
 * Lab 3 Exercise 3 Priority
 * Name: She Jiayu
 * Student No: A0188314B
 * Lab Group: 9
 *************************************/

#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>

#include "traffic_synchronizer.h"

//Using extern, you can use the global variables num_of_cars and num_of_segments from ex3_runner.c in your code.
extern int num_of_cars;
extern int num_of_segments;

sem_t *segment_locks;
sem_t *segment_priority_locks;
sem_t *segment_priority_mutex;
int *segment_priority_count;
sem_t cars_in_loop;

void initialise()
{
    segment_locks = malloc(sizeof(sem_t) * num_of_segments);
    segment_priority_locks = malloc(sizeof(sem_t) * num_of_segments);
    for (int i = 0; i < num_of_segments; i++) {
        sem_init(&segment_locks[i], 0, 1);
        sem_init(&segment_priority_locks[i], 0, 1);
    }
    sem_init(&cars_in_loop, 0, num_of_segments - 1);
}

void cleanup()
{
    for (int i = 0; i < num_of_segments; i++) {
        sem_destroy(&segment_locks[i]);
        sem_destroy(&segment_priority_locks[i]);
    }
    sem_destroy(&cars_in_loop);
    free(segment_locks);
    free(segment_priority_locks);
}

void* car(void* car)
{
    //This function models a thread
    //A car:
    //   -should call enter_roundabout (...)
    //   -followed by some calls to move_to_next_segment (...)
    //   -finally call exit_roundabout (...)
    car_struct* carNow = (car_struct*) car;
    int entry_no = carNow->entry_seg;
    int exit_no = carNow->exit_seg;
    sem_wait(&cars_in_loop);
    sem_wait(&segment_priority_locks[entry_no]);
    sem_wait(&segment_locks[entry_no]);
    sem_wait(&segment_priority_locks[NEXT(entry_no, num_of_segments)]);
    enter_roundabout(carNow);
    sem_post(&segment_priority_locks[entry_no]);
    while (carNow->current_seg != exit_no) {
        int curr_no = carNow->current_seg;
        int next_no = NEXT(curr_no, num_of_segments);
        sem_wait(&segment_locks[next_no]);
        if (next_no != exit_no) {
            sem_wait(&segment_priority_locks[NEXT(next_no, num_of_segments)]);
        }
        move_to_next_segment(carNow);
        sem_post(&segment_priority_locks[next_no]);
        sem_post(&segment_locks[curr_no]);
    }
    exit_roundabout(carNow);
    sem_post(&segment_locks[exit_no]);
    sem_post(&cars_in_loop);
    pthread_exit(NULL);
}
