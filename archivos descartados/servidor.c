#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>   /* For mode constants */
#include <string.h>
#include <pthread.h>    /* For threads */
#include <signal.h>    /* For signal handling */
#include <sys/socket.h> /* For sockets */
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>    /* For INT_MAX */
#include <sys/time.h>   /* For timeval structure */

#include "funciones_servidor/funciones_servidor.h"
#include "funciones_sockets/funciones_sockets.h"


int server_sd, client_sd;                        // Server and client socket descriptors
char buffer[10706];                                 // Buffer for the messages

pthread_cond_t cond_message;    // Condition variable to signal the arrival of a new message
pthread_mutex_t mutex_message;  // Mutex to protect the access to the message
int processed_request = 0;      // Flag to indicate if the request has been processed

int CLIENT_TIMEOUT_SEC = 5;     // Timeout for the client socket (seconds)
int CLIENT_TIMEOUT_USEC = 0;    // Timeout for the client socket (microseconds)

void end(int sig){
    // Signal handler for the SIGINT signal (Ctrl+C)

    printf("Exiting the server...\n");

    // Close the server socket
    close(server_sd);

    // Close the client socket
    close(client_sd);

    // Exit the program    
    exit(0);
}

int process_request(Request *request){
    // Process the request (do the operations stated in the request and send the response)

    char response_buffer[10695];    // Buffer for the response

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

    // Parse the response to the buffer
    if (request_copy.op == GET_VALUE){
        // Copy the error code, value1 and N_value2 to the buffer
        sprintf(response_buffer, "%d %s %d", response.res, response.value1, response.N_value2);

        // Add the values of the vector V_value2 to the buffer next to the N_value2
        for (int i = 0; i < response.N_value2; i++)
        {
            sprintf(response_buffer + strlen(response_buffer), " %lf", response.V_value2[i]);
        }
    } else {
        sprintf(response_buffer, "%d", response.res);
    }

    // Send the response
    if (sendMessage(request_copy.client_sd, response_buffer, strlen(response_buffer) + 1) == -1){
        perror("Error sending the response\n");
        return -1;
    }

    // Close the client socket
    close(request_copy.client_sd);
    return 0;
}

int parse_request(char *buffer, Request *request){
    // Parse the request from the buffer
    // printf("Parsing request\n");
    char *token;
    int i = 0;
    char *endptr;

    // Get the first token
    token = strtok(buffer, " ");
    while (token != NULL)
    {   
        if (token[0] != '\0') {
            // printf("i: %d, token: %s\n", i, token);
            switch (i)
            {
                case 0: // The first token is the operation code
                    errno = 0;
                    long op = strtol(token, &endptr, 10);
                    if (errno != 0 || *endptr != '\0' || op < INIT || op > EXIST){
                        perror("Invalid operation code\n");
                        return -1;
                    }
                    request->op = (int)op;
                    break;
                case 1: // The second token is the key
                    errno = 0;
                    long key = strtol(token, &endptr, 10);
                    if (errno != 0 || *endptr != '\0' || key < 0 || key > INT_MAX){
                        perror("Invalid key\n");
                        return -1;
                    }
                    request->key = (int)key;
                    break;
                case 2: // ...
                    strcpy(request->value1, token);
                    break;
                case 3:
                    errno = 0;
                    long N_value2 = strtol(token, &endptr, 10);
                    if (errno != 0 || *endptr != '\0' || N_value2 < 0 || N_value2 > 32){
                        perror("Invalid N_value2\n");
                        return -1;
                    }
                    request->N_value2 = (int)N_value2;
                    break;
                default:
                    // If the current index is greater than N_value2, throw an error
                    if (i > request->N_value2 + 3){
                        perror("Invalid number of values\n");
                        return -1;
                    }
                    errno = 0;
                    double V_value2 = strtod(token, &endptr);
                    if (errno != 0 || *endptr != '\0'){
                        perror("Invalid V_value2 element\n");
                        return -1;
                    }
                    request->V_value2[i - 4] = V_value2;
                    break;
            }
        }
        i++;
        token = strtok(NULL, " ");
    }
    // printf("Request parsed\n");
    return 0;
}


int main(int argc, char *argv[])
{
    signal (SIGINT, end);

    pthread_t thread_id;
    pthread_attr_t t_attr;

    struct sockaddr_in server_addr, client_addr = {0};  // Server and client addresses
    socklen_t client_addr_len = sizeof(client_addr);    // Length of the client address
    
    char *port;                                     // Server port number

    // Timeout for the client socket
    struct timeval timeout;
    timeout.tv_sec = CLIENT_TIMEOUT_SEC;
    timeout.tv_usec = CLIENT_TIMEOUT_USEC;

    // Check the number of arguments
    if (argc != 2){
        printf("Incorrect number of arguments. Usage: %s <port>\n", argv[0]);
        return -1;
    }

    // Get the port number
    port = argv[1];

    // Create the server socket
    if ((server_sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
        perror("Error creating the server socket\n");
        return -1;
    }

    // Set the SO_REUSEADDR option
    int enable = 1;
    if (setsockopt(server_sd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1){
        perror("Error setting the SO_REUSEADDR option\n");
        return -1;
    }
    // Fill the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port));
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);    // Listen on any address

    // Bind the server socket to the server address
    if (bind(server_sd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
        perror("Error binding the server socket\n");
        close(server_sd);
        return -1;
    }

    // Initialize the mutex and condition variable
    pthread_mutex_init(&mutex_message, NULL);
    pthread_cond_init(&cond_message, NULL);
    pthread_attr_init(&t_attr); // IMPORTANT: Initialize the thread attributes (the thread creation failed sometimes without this line)
    pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);

    // Listen for connections
    if (listen(server_sd, SOMAXCONN) == -1){    // SOMAXCONN is the maximum number of pending connections (1024 by default)
        perror("Error listening for connections\n");
        close(server_sd);
        return -1;
    }

    // Keep listening for requests
    while (1)
    {
        Request request;

        printf("Waiting for a connection...\n");

        // Connect with the client
        client_sd = accept(server_sd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sd == -1){
            perror("Error accepting the connection\n");
            close(client_sd);
            continue;
        }

        printf("Connection accepted from IP: %s, Port: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Set the timeout for receive operations on the client socket

        if (setsockopt(client_sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
            perror("Error setting the timeout for the client socket\n");
            close(client_sd);
            continue;
        }

        // printf("Timeout set\n");

        // Receive the request
        if (readLine(client_sd, buffer, sizeof(buffer)) == -1){
            perror("Error receiving the request\n");
            close(client_sd);
            continue;
        }

        // printf("Request received: %s\n", buffer);

        // Parse the request
        if (parse_request(buffer, &request) == -1){
            perror("Error parsing the request\n");
            close(client_sd);
            printf("Client disconnected\n");
            continue;
        }

        // printf("Request parsed\n");

        // Copy the client_sd to the request
        request.client_sd = client_sd;

        // printf("Request received: op = %d, key = %d, value1 = %s, N_value2 = %d\n", request.op, request.key, request.value1, request.N_value2);

        // Create a new thread to process the request
        if (pthread_create(&thread_id, &t_attr, (void *)process_request, &request) != 0){
            perror("Error creating the thread\n");
            continue;
        }

        // printf("Thread created\n");

        // Lock the mutex
        pthread_mutex_lock(&mutex_message);
        while (!processed_request)
        {
            pthread_cond_wait(&cond_message, &mutex_message);
        }
        processed_request = 0;
        pthread_mutex_unlock(&mutex_message);

        // printf("Request processed\n");
    }
}
