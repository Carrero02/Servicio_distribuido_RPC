#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>   /* For mode constants */
#include <string.h>
#include <mqueue.h>     /* For Message Queues */
#include <pthread.h>    /* For threads */
#include <signal.h>    /* For signal handling */

#include "mensaje.h"
#include "funciones_servidor/funciones_servidor.h"


struct mq_attr requestAttributes = {
    .mq_flags = 0,                  // Flags (ignored for mq_open())
    .mq_maxmsg = 10,                // Max. # of messages on queue (the server can receive up to 10 requests at the same time)
    .mq_msgsize = sizeof(Request),  // Max. message size (in bytes)
    .mq_curmsgs = 0                 // # of messages currently in queue (ignored for mq_open())
};


mqd_t server_queue;
pthread_cond_t cond_message;    // Condition variable to signal the arrival of a new message
pthread_mutex_t mutex_message;  // Mutex to protect the access to the message
int processed_request = 0;         // Flag to indicate if the request has been processed

void end(int sig){
    // Close the server queue
    if (mq_close(server_queue) == -1){
        perror("Error closing the server queue\n");
    }
    // Unlink the server queue
    if (mq_unlink(SERVER_QUEUE_NAME) == -1){
        perror("Error unlinking the server queue\n");
    }
    exit(0);
}

int process_request(Request *request){
    // Process the request (do the operations stated in the request and send the response)

    // Lock the mutex
    pthread_mutex_lock(&mutex_message);
    
    Response response;
    // Copy the request to have a local copy of it
    Request request_copy = *request;

    // Process the request
    switch (request_copy.op)
    {
        case INIT:
            response.res = init();
            break;
        case SET_VALUE:
            response.res = set_value(request_copy.key, request_copy.value1, request_copy.N_value2, request_copy.V_value2);
            break;
        case GET_VALUE:
            // Note: get_value() modifies response.value1, response.N_value and response.V_value2
            response.res = get_value(request_copy.key, response.value1, &response.N_value2, response.V_value2);
            break;
        case MODIFY_VALUE:
            response.res = modify_value(request_copy.key, request_copy.value1, request_copy.N_value2, request_copy.V_value2);
            break;
        case DELETE_KEY:
            response.res = delete_key(request_copy.key);
            break;
        case EXIST:
            response.res = exist(request_copy.key);
            break;
        default:
            response.res = -1;
            break;
    }

    processed_request = 1;
    pthread_cond_signal(&cond_message);
    pthread_mutex_unlock(&mutex_message);

    mqd_t client_queue;

    // Open the client queue
    client_queue = mq_open(request_copy.q_name, O_WRONLY);
    if (client_queue == -1){
        perror("Error opening the client queue\n");
        // mq_close(server_queue);  // We do not close the server queue, we just continue with the next client petition
        return -1;
    }

    // Send the response
    if (mq_send(client_queue, (const char *)&response, sizeof(Response), 0) == -1){
        perror("Error sending the response\n");
        // mq_close(server_queue);
        mq_close(client_queue);
        return -1;
    }

    // Close the client queue
    // If it is not closed, at some point we will get "Error creating the client queue: No space left on device"
    // When creating the client queue
    mq_close(client_queue);

    return 0;
}
   

int main(int argc, char *argv[])
{
    signal (SIGINT, end);

    pthread_t thread_id;
    pthread_attr_t t_attr;

    // Open the server queue
    server_queue = mq_open(
                            SERVER_QUEUE_NAME, 
                            O_CREAT | O_RDONLY, // The server has to read the request from the server queue
                            S_IRUSR | S_IWUSR,  // Read by user, write by user (user owner of the queue file)
                            &requestAttributes
                            );

    if (server_queue == -1){
        perror("Error opening the server queue\n");
        return -1;
    }

    // Initialize the mutex and condition variable
    pthread_mutex_init(&mutex_message, NULL);
    pthread_cond_init(&cond_message, NULL);
    pthread_attr_init(&t_attr); // IMPORTANT: Initialize the thread attributes (the thread creation failed sometimes without this line)
    pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);

    // Keep listening for requests
    while (1)
    {
        Request request;

        printf("Waiting for a request...\n");

        // Receive the request
        if (mq_receive(server_queue, (char *)&request, sizeof(Request), NULL) == -1){
            perror("Error receiving the request\n");
            mq_close(server_queue);
            return -1;
        }

        printf("Received a request\n");

        // Create a new thread to process the request
        if (pthread_create(&thread_id, &t_attr, (void *)process_request, &request) != 0){
            perror("Error creating the thread\n");
            continue;
        }

        // Lock the mutex
        pthread_mutex_lock(&mutex_message);
        while (!processed_request)
        {
            pthread_cond_wait(&cond_message, &mutex_message);
        }
        processed_request = 0;
        pthread_mutex_unlock(&mutex_message);
        printf("Request processed\n");
    }
}
