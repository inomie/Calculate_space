#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "queue.h"
#include "list.h"

/**
 * @author Robin Lundin Söderberg
 * @Version: 1.0
 * @Date: 2021-11-11 
 * 
 * All the functions exept from queue and list functions.
 * 
 */

// ===========INTERNAL DATA TYPES============

/*
 * The queue is implemented using the list abstract datatype. Almost
 * everything is done by the list.
 */

struct queue {
	list *elements;
};

pthread_mutex_t mutex;
pthread_cond_t condition;


// ===========INTERNAL FUNCTION IMPLEMENTATIONS============

/**
 * The function will init mutex and conditional variable.
 * 
 */
void init_mutex_cond(void) {
	
	if (pthread_mutex_init(&mutex, NULL) != 0) {
		perror("Failed to init mutex in queue!");
		exit(EXIT_FAILURE);
	}

	if (pthread_cond_init(&condition, NULL) != 0) {
		perror("Failed init conditional variable in the queue!");
		exit(EXIT_FAILURE);
	}
}

/**
 * The function will create a empty list (queue).
 * But are doing it thread safe.
 * @param free_func NULL for defeault
 * @return queue* that was created
 * 
 */
queue *queue_empty(free_function free_func)
{	
	
	init_mutex_cond();

	// Allocate the queue head.
	queue *q=calloc(1, sizeof(*q));

	if (q == NULL) {
		perror("Failed to allocate");
		exit(EXIT_FAILURE);
	}

	// Create an empty list.
	q->elements=list_empty(free_func);

	return q;
}

/**
 * The function checks if the threads is done and if the queue is empty.
 * But are doing it thread safe.
 * @param q queue 
 * @param sem semaphore to check
 * @param threads numbers of threads
 * @return an bool value
 * 
 */
bool done(const queue *q, int threads, sem_t *sem) {	

	pthread_mutex_lock(&mutex);

	/* Get the semaphore value to see if all are done */
	int sem_value;
	sem_getvalue(sem, &sem_value);
	
	/* If the queue is empty and the threads is not done it's going to wait */
	while (list_is_empty(q->elements) && (sem_value != threads)) {
		pthread_cond_wait(&condition, &mutex);
		sem_getvalue(sem, &sem_value);
	}

	/* If the list is empty and all the threads is done */
	if (list_is_empty(q->elements) && (sem_value == threads)) {	
		
		pthread_cond_signal(&condition);
		pthread_mutex_unlock(&mutex);
		
		return true;
	}
	
	pthread_mutex_unlock(&mutex);

	return false;
}


/**
 * The function adds entity to the queue. But are doing it thread safe.
 * @param q is the queue.
 * @param entity is the entity thats going to be added to the queue.
 * 
 */
void queue_enqueue(queue *q, void *entity) {	

	pthread_mutex_lock(&mutex);

	/* Add entity to the list */
	list_insert(q->elements, entity, list_end(q->elements));

	/* Sending signal to say that a entity has been added */ 
	pthread_cond_signal(&condition);

	pthread_mutex_unlock(&mutex);

}

/**
 * The function will get the first entity in line and remove it.
 * But are doing it thread safe.
 * @param q is the queue to get the entity from.
 * @return char pointer to the entity that was first in line
 * in the queue.
 * 
 */
char *queue_dequeue(queue *q) {	
	
	pthread_mutex_lock(&mutex);

	if(!list_is_empty(q->elements)) {

		char *entity = NULL;

		/* Get the entity that is first in line in the queue */
		entity = list_inspect(q->elements, list_first(q->elements));

		/* Remove the one first in line */
		list_remove(q->elements, list_first(q->elements));

		pthread_mutex_unlock(&mutex);
		return entity;
	}

	pthread_mutex_unlock(&mutex);
	return NULL;
	
}

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
void queue_kill(queue *q)
{	
	/* Destroy the list */
	list_kill(q->elements);
	
	pthread_cond_destroy(&condition);
	pthread_mutex_destroy(&mutex);

	free(q);
}
