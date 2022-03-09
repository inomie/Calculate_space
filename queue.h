#ifndef __QUEUE_H
#define __QUEUE_H

#include <semaphore.h>
#include <stdbool.h>

#include "util.h"

/**
 * @author Robin Lundin Söderberg
 * @Version: 1.0
 * @Date: 2021-11-15 
 * 
 * All the functions exept from queue and list functions.
 * 
 */

// ==========PUBLIC DATA TYPES============

// Queue type.
typedef struct queue queue;

// ==========DATA STRUCTURE INTERFACE==========

/**
 * The function will init mutex and conditional variable.
 * 
 */
void init_mutex_cond(void);

/**
 * The function will create a empty list (queue).
 * But are doing it thread safe.
 * @param free_func NULL for defeault
 * @return queue* that was created
 * 
 */
queue *queue_empty(free_function free_func);

/**
 * The function checks if the threads is done and if the queue is empty.
 * But are doing it thread safe.
 * @param q queue 
 * @param sem semaphore to check
 * @param threads numbers of threads
 * @return an bool value
 * 
 */
bool done(const queue *q, int threads, sem_t *sem);


/**
 * The function adds entity to the queue. But are doing it thread safe.
 * @param q is the queue.
 * @param entity is the entity thats going to be added to the queue.
 * 
 */
void queue_enqueue(queue *q, void *entity);

/**
 * The function will get the first entity in line and remove it.
 * But are doing it thread safe.
 * @param q is the queue to get the entity from.
 * @return char pointer to the entity that was first in line
 * in the queue.
 * 
 */
char *queue_dequeue(queue *q);

/**
 * queue_kill() - Destroy a given queue.
 * @q: Queue to destroy.
 *
 * Return all dynamic memory used by the queue and its elements. If a
 * free_func was registered at queue creation, also calls it for each
 * element to free any user-allocated memory occupied by the element values.
 *
 * Returns: Nothing.
 */
void queue_kill(queue *q);

#endif
