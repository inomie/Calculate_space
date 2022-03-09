#include "data.h"

/**
 * @author Robin Lundin Söderberg
 * @Version: 2.0
 * @Date: 2021-11-11 
 * 
 * The program Will calculate the disc space of folders and files.
 * To run the program (./mdu [-j] [number of threads] [file...][Folder...])
 * or (./mdu [file...][folder...]). The program will calculate how many blockes
 * it will take on the disc. If the [-j] flag is uses it will calculate it faster
 * when used [threads (7-8)].
 * 
 */

int main(int argc, char *argv[]) {
    
    if (argc < 2) {
        fprintf(stderr, "To few arguments, need files or directories!\n");
        return EXIT_FAILURE;
    }
    
    Data *data;

    /* Allocate the struct */
    if((data = malloc(sizeof(*data))) == NULL) {
        perror("Malloc failed!");
        return EXIT_FAILURE;
    }

    /* Init the values to the variables in the struct */
    data_Init(data);

    /* Check and see if the flag has been used and how many threads */
    get_Flags(data, argc, argv);

    /* Get the entity from the argument and add it to the queue */
    add_entity_to_queue(data, argc ,argv);

    queue_kill(data->queue);
    pthread_mutex_destroy(&data->mutex);

    /* Set the exit status to an variable so it can be free'd */
    int retValue = data->exit_status;
    free(data); 
    return retValue;
}

