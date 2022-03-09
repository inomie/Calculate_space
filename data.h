#ifndef __DATA_H
#define __DATA_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include "queue.h"
#include "list.h"
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>
#include <time.h>

/**
 * @author Robin Lundin Söderberg
 * @Version: 2.0
 * @Date: 2021-11-17
 * 
 * All the functions declaration exept from queue and list functions.
 * 
 */

typedef struct Data {   
    int threads;
    int exit_status;
    queue *queue;
    pthread_mutex_t mutex;
    sem_t sem;
}Data;

/**
 * The function will search for the [-j] flag and if it finds it
 * it will get the number of threads.
 * @param data The struct of all the variables that threads can be needing.
 * @param argc Number of arguments.
 * @param argv Array of all the arguments.
 * 
 */
void get_Flags(Data *data, int argc, char *argv[]);

/**
 * The function will join each thread
 * and add up the size.
 * @param th An array of the thread's.
 * @param data is a struct that cointains all the variable's.
 * @return the size value.
 * 
 */
int joinThreads(pthread_t th[], Data *data);

/**
 * The function will get the information of the entity.
 * @param entity The folder or file that's going to be check.
 * @return Will return an number of blocks that entity take's
 * on the disc space.
 * 
 */
int get_entity_size(const char *entity);

/**
 * The function will open an directory and then add all
 * the thing's it's contains to the queue.
 * @param directory that's going to get open.
 * @param data is the struct with all the variable's.
 * 
 */
void open_directory(const char *directory, Data *data, int *size);

/**
 * The function will get the mode of the entity.
 * @param entiry The folder or file.
 * @return the mode of the the entity.
 */
mode_t get_entity_mode(const char *entity);

/**
 * Going to create all the thread's that was given
 * with the [-j] flag at start.
 * @param th An array of all the thread's.
 * @param data is a struct that cointains all the variable's.
 * 
 */
void createThreads(pthread_t th[], Data *data);

/**
 * The function is to get entity's from the queue and
 * check what mode it is. Depends if the entity is a 
 * file/link or a folder it will do diffrent thing's.
 * Is it a file/link will it get the size of it other
 * wise it will open it up and get all the folders/files
 * it contains and add it to the queue and then get the
 * size of the folder.
 * @param ptr is a void pointer to the data struct.
 * @return a void pointer but it is the size.
 * 
 */
void *get_entity_from_queue(void *ptr);

/**
 * The function will create all the threads and then join them
 * to get the size.
 * @param data is a struct that cointains all the variable's.
 * @return the size value.
 * 
 */
int thread_create_join(Data *data);

/**
 * The function will init som variables in the struct data.
 * @param data is a struct that cointains all the variable's.
 * 
 */
void data_Init(Data *data);

/**
 * The function will init the semaphore that data cointains.
 * @param data is a struct that cointains all the variable's.
 * 
 */
void semaphore_init(Data *data);

/**
 * The function will add the arguments to the queue and go through them.
 * @param data is a struct that cointains all the variable's.
 * @param argc is the number of argument's.
 * @param argv is the array that contains the argument's.
 * 
 */
void add_entity_to_queue(Data *data, int argc, char *argv[]);

/**
 * The function will print out the size of the argument.
 * @param size is the size of the entity.
 * @param arg is the argument.
 * 
 */
void print(int size, char *arg);

/**
 * The function will check how many threads that's going to do work.
 * If it's not more then 1 thread it's going to create and join threads.
 * @param data is a struct that cointains all the variable's.
 * @param argv is the array that contains the argument's.
 * @param i is the number of the argument right now.
 * 
 */
void number_of_threads(Data *data, char *argv[], int i);

#endif