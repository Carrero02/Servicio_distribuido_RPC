#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>    /* For signal handling */
#include <time.h>      /* For time() */

#include "claves/claves.h"


void end(int sig){
    // Unlink the client queue
    char client_queue_name[256];
    sprintf(client_queue_name, "/mq_client_%u", getpid());
    if (mq_unlink(client_queue_name) == -1){
        perror("Error unlinking the client queue");
    }
    exit(0);  
}

int main(int argc, char *argv[])
{
    signal (SIGINT, end);
    srand(time(NULL));  // Seed for the random number generator based on the current time

    int key;
    char value1[256];
    int N_value2;
    double V_value2[32];

    char value1_get[256];
    int N_value2_get;
    double V_value2_get[32];

    // Infinite loop
    while (1)
    {
        // If the file "tuplas.txt" does not exist, run init()
        FILE *file = fopen("tuplas.txt", "r");
        if (file == NULL)
        {
            init();
        }
        else
        {
        fclose(file);
        }

        // Try to add a key
        // If it already exists, delete it and try again
        key = rand() % 1000;    // Random key between 0 and 999
        sprintf(value1, "value1_%d", key);
        N_value2 = rand() % 32 + 1; // Random number between 1 and 32
        for (int i = 0; i < N_value2; i++)
        {
            V_value2[i] = (double)rand() / RAND_MAX;
        }
        while (set_value(key, value1, N_value2, V_value2) == -1)
        {
            delete_key(key);
        }
        // Sleep one second
        // sleep(1);

        // Try to get the recently added key
        // If it does not exist, try again
        while (get_value(key, value1_get, &N_value2_get, V_value2_get) == -1)
        {
            set_value(key, value1, N_value2, V_value2);
        }
        // Sleep one second
        // sleep(1);

        // Try to modify the recently added key
        // If it does not exist, use set_value()
        sprintf(value1, "value1_%d", key);
        N_value2 = rand() % 32 + 1;
        for (int i = 0; i < N_value2; i++)
        {
            V_value2[i] = (double)rand() / RAND_MAX;
        }
        while (modify_value(key, value1, N_value2, V_value2) == -1)
        {
            set_value(key, value1, N_value2, V_value2);
        }
        // Sleep one second
        // sleep(1);

        // Try to get the recently modified key
        // If it does not exist, try again
        while (get_value(key, value1_get, &N_value2_get, V_value2_get) == -1)
        {
            set_value(key, value1, N_value2, V_value2);
        }
        // Sleep one second
        // sleep(1);
    }

    return 0;
}