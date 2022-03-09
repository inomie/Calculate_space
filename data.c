#include "data.h"

/**
 * @author Robin Lundin Söderberg
 * @Version: 2.0
 * @Date: 2021-11-17
 * 
 * All the functions exept from queue and list functions.
 * 
 */

/**
 * The function will search for the [-j] flag and if it finds it
 * it will get the number of threads.
 * @param data The struct of all the variables that threads can be needing.
 * @param argc Number of arguments.
 * @param argv Array of all the arguments.
 * 
 */
void get_Flags(Data *data, int argc, char *argv[]) {

    char flag;

    /* Loop through to find the [-j] flag and number of threads */
    while ((flag = getopt(argc, argv, "j:")) != -1) {   
        if (flag == 'j') {  

            /* Get number of threads */ 
            data->threads = atoi(optarg);

            if (data->threads < 1) {
                fprintf(stderr, "Invalid amount of threads, need atleast one!\n");
                exit(EXIT_FAILURE);
            }
        } else {
            fprintf(stderr, "No valid flag was given, flags(-j)!\n");
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * The function will get the mode of the entity.
 * @param entiry The folder or file.
 * @return the mode of the the entity.
 */
mode_t get_entity_mode(const char *entity) {
    struct stat file_info;
    
    if (lstat(entity, &file_info) < 0) {
        perror(entity);
        exit(EXIT_FAILURE);
    }

    return file_info.st_mode;
}

/**
 * The function will get the information of the entity.
 * @param entity The folder or file that's going to be check.
 * @return Will return an number of blocks that entity take's
 * on the disc space.
 * 
 */
int get_entity_size(const char *entity) {  

    struct stat file_info;
    
    if (lstat(entity, &file_info) < 0) {
        perror(entity);
        exit(EXIT_FAILURE);
    }
    
    return file_info.st_blocks;
}

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
void *get_entity_from_queue(void *ptr) {

    /* Cast from void* to Data* */ 
    Data *data = (Data *)ptr;

    char *entity;
    int *size = malloc(sizeof(*size));
    *size = 0;
    
    /* Loop until the queue is done */
    while (!done(data->queue, data->threads, &data->sem)) {   
        sem_wait(&data->sem);

        /* Get entity from queue */
        entity = queue_dequeue(data->queue);
        if(entity != NULL) {
            
            open_directory(entity, data, size);
            *size += get_entity_size(entity);  
            
            free(entity);
        }
        sem_post(&data->sem);
    }
    return size;
}

/**
 * The function will open an directory and then add all
 * the thing's it's contains to the queue.
 * @param directory that's going to get open.
 * @param data is the struct with all the variable's.
 * 
 */
void open_directory(const char *directory, Data *data, int *size) {

    /* Open the directory */
    DIR *dr = opendir(directory);

    /* Controll so it can open the folder */
    if (dr == NULL) {
        fprintf(stderr, "du: cannot read directory '%s': Permission denied\n", directory);
        pthread_mutex_lock(&data->mutex);
        data->exit_status = EXIT_FAILURE;
        pthread_mutex_unlock(&data->mutex);
        return;
    }  

    struct dirent* dirent;
    mode_t entity_mode;

    /* Loop through the folder */
    while ((dirent = readdir(dr)) != NULL) {   
        
        if ((strcmp(dirent->d_name, ".") != 0) && (strcmp(dirent->d_name, "..") != 0)) {
            
            char *entity = malloc(4096*sizeof(*entity));

            if (entity == NULL) {
                perror("Failed to allocate!");
                exit(EXIT_FAILURE);
            }

            entity[0] = '\0';

            /* Copy the folder name */
            strcat(entity, directory);

            /* Check that it's not ending with '/' already */
            if (entity[strlen(entity) -1] != '/') {
                strcat(entity, "/");
            }

            /* Copy over the folder/file name */
            strcat(entity, dirent->d_name);

            entity_mode = get_entity_mode(entity);

            /* Check what mode the entity is */
            if (S_ISREG(entity_mode) || S_ISLNK(entity_mode)) {
                *size += get_entity_size(entity);
                free(entity);       
            } else if (S_ISDIR(entity_mode)) {
                queue_enqueue(data->queue, entity);  
            } 
        }  
    }
    closedir(dr); 
}

/**
 * Going to create all the thread's that was given
 * with the [-j] flag at start.
 * @param th An array of all the thread's.
 * @param data is a struct that cointains all the variable's.
 * 
 */
void createThreads(pthread_t th[], Data *data) {
    for (int i = 0; i < data->threads; i++) {   
        if (pthread_create(&th[i], NULL, *get_entity_from_queue, data) != 0) {
            perror("Failed to create thread!");
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * The function will join each thread
 * and add up the size.
 * @param th An array of the thread's.
 * @param data is a struct that cointains all the variable's.
 * @return the size value.
 * 
 */
int joinThreads(pthread_t th[], Data *data) {

    int size = 0;

    /* Used to get the size when joining thread */
    int *size_catch;

    for (int i = 0; i < data->threads; i++) {
        if (pthread_join(th[i], (void **)&size_catch) != 0) {
            perror("Failed to join thread!");
            exit(EXIT_FAILURE);
        }

        size += *size_catch;
        free(size_catch);
    }

    return size;
}

/**
 * The function will create all the threads and then join them
 * to get the size.
 * @param data is a struct that cointains all the variable's.
 * @return the size value.
 * 
 */
int thread_create_join(Data *data) {

    /* All the threads */
    pthread_t th[data->threads];

    /* Create all the threads*/
    createThreads(th, data);

    /* Join all the threads and then return the return value */
    return joinThreads(th, data);
}

/**
 * The function will init som variables in the struct data.
 * @param data is a struct that cointains all the variable's.
 * 
 */
void data_Init(Data *data) {
    /* Start value of threads */
    data->threads = 1;
    /* Used to be able to see if something fail's */
    data->exit_status = EXIT_SUCCESS;

    /* Init of the mutex */
	if (pthread_mutex_init(&data->mutex, NULL) != 0) {
		perror("Mutex failed!");
		exit(EXIT_FAILURE);
	}
}

/**
 * The function will add the arguments to the queue and go through them.
 * @param data is a struct that cointains all the variable's.
 * @param argc is the number of argument's.
 * @param argv is the array that contains the argument's.
 * 
 */
void add_entity_to_queue(Data *data, int argc, char *argv[]) {
    
    /* Make the queue */
    data->queue = queue_empty(NULL);
    mode_t entity_mode;
    /* Loop through the arguments */
    for (int i = optind; i < argc ; i++) {   

        semaphore_init(data);

        entity_mode = get_entity_mode(argv[i]);

        /* Check what mode the entity is */
        if (S_ISREG(entity_mode) || S_ISLNK(entity_mode)) {
            int size = get_entity_size(argv[i]); 
            print(size, argv[i]);     
        } else {
            /* Add the argument to the queue */
            queue_enqueue(data->queue, strdup(argv[i]));

            /* See how many threads it is that's going to do work */
            number_of_threads(data, argv, i);

            /* Destroy the semaphore to be able to make a new one */
            sem_destroy(&data->sem);

        }
        
        
    }
}

/**
 * The function will check how many threads that's going to do work.
 * If it's not more then 1 thread it's going to create and join threads.
 * @param data is a struct that cointains all the variable's.
 * @param argv is the array that contains the argument's.
 * @param i is the number of the argument right now.
 * 
 */
void number_of_threads(Data *data, char *argv[], int i) {
    /* Check if it's more then one thread else do the calculation */
    if (data->threads > 1) {
        int size = thread_create_join(data);
        print(size, argv[i]);
    } else {   
        int *size = get_entity_from_queue(data);
        int data_size = *size;
        free(size);
        print(data_size, argv[i]);
    }
}

/**
 * The function will print out the size of the argument.
 * @param size is the size of the entity.
 * @param arg is the argument.
 * 
 */
void print(int size, char *arg) {
    fprintf(stdout, "%d\t", size);
    fprintf(stdout, "%s\n", arg);
}

/**
 * The function will init the semaphore that data cointains.
 * @param data is a struct that cointains all the variable's.
 * 
 */
void semaphore_init(Data *data) {
    /* Init the semaphore with the numbers of threads */
    if (sem_init(&data->sem, 0, data->threads) == -1) {
        perror("Sem init failed!");
        exit(EXIT_FAILURE);
    }
}