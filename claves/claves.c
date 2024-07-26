/*
Code that implements the functions defined in claves.h
*/

#include "claves.h"


struct mq_attr responseAttributes = {
    .mq_flags = 0,
    .mq_maxmsg = 1,                 // Max. # of messages on queue (the client can receive up to 1 response at the same time)
    .mq_msgsize = sizeof(Response), // TODO: Define Response
    .mq_curmsgs = 0
};

mqd_t server_queue, client_queue;

int open_queues()
{
    // Open the server queue
    // NOTE: When the queue is opened (and not created), we only pass 2 arguments (queue name and flags)
    server_queue = mq_open(
                            SERVER_QUEUE_NAME, 
                            O_WRONLY // The client has to write the request to the server queue
                            );

    if (server_queue == -1){
        perror("Error opening the server queue\n");
        return -1;
    }

    // Create the client queue
    char client_queue_name[256];
    sprintf(client_queue_name, "/mq_client_%u", getpid());
    // NOTE: When the queue is created, we pass 4 arguments (queue name, flags, permissions and attributes)
    client_queue = mq_open(
                            client_queue_name,
                            O_CREAT | O_RDONLY, // The client only has to read the response from its queue
                            S_IRUSR | S_IWUSR,  // Read by user, write by user (user owner of the queue file)
                            &responseAttributes
                            );

    if (client_queue == -1){
        perror("Error creating the client queue\n");
        return -1;
    }

    return 0;
}

int send_message(Request* request){
    int retries = 0;
    //Función que envía un mensaje a la cola del servidor
    while (mq_send(server_queue, (const char *)request, sizeof(Request), 0) == -1){
        // If the server queue was full, the concurrent clients failed to send the message
        if (errno == EAGAIN){
            printf("Server queue is full, trying again...\n");
            sleep(1 << retries); // Exponential backoff (2^retries seconds)
            retries++;
            if (retries > MAX_RETRIES){
                perror("Error sending the message\n");
                return -1;
            }
        }
        else{
            perror("Error sending the message\n");
            return -1;
        }
    }
    return 0;
}

int receive_message(Response* response){
    // Función que recibe un mensaje en la cola del cliente
    if (mq_receive(client_queue, (char *)response, sizeof(Response), 0) == -1){
        perror("Error receiving the message\n");
        return -1;
    }
    return 0;
}

int init(){
    // Inicializamos el servicio de elementos clave-valor1-valor2
    // Destruimos todas las tuplas que estuvieran almacenadas previamente
    // Devuelve 0 en caso de éxito y -1 en caso de error.
    
    open_queues();

    // Create the request
    Request request;
    // Create the response
    Response response;
    
    // Fill the request
    request.op = 0;
    char client_queue_name[256];
    sprintf(client_queue_name, "/mq_client_%u", getpid());
    strcpy(request.q_name, client_queue_name);

    // Send the request
    if (send_message(&request) < 0) { return -1; }

    // Receive the response
    if (receive_message(&response) < 0) { return -1; }

    // Close the queues
    mq_close(server_queue);
    mq_close(client_queue);
    mq_unlink(client_queue_name);

    //Return the response
    return response.res;
}

int set_value(int key, char *value1, int N_value2, double *V_value2){
    // Almacena la tupla (key, value1, value2) en el servicio de elementos clave-valor1-valor2
    // Si ya existía una tupla con clave key, se sobreescribe con la nueva tupla
    // Devuelve 0 en caso de éxito y -1 en caso de error.

    // Handling errors in arguments

    // If any argument is NULL, we return -1
    if (value1 == NULL || V_value2 == NULL){
        return -1;
    }

    // if value1 has more tha 256 Bytes, we return -1
    if (strlen(value1) > 256){
        return -1;
    }
    
    // If N_value2 is not between 1 and 32, we return -1
    if (N_value2 < 1 || N_value2 > 32){
        return -1;
    }

    // If V_value2 has more than 32 elements, we return -1
    if (sizeof(V_value2) > 32){
        return -1;
    }

    open_queues();

    // Create the request
    Request request;
    // Create the response
    Response response;

    // Fill the request
    request.op = 1;
    request.key = key;
    strcpy(request.value1, value1);
    request.N_value2 = N_value2;
    memcpy(request.V_value2, V_value2, N_value2 * sizeof(double));
    char client_queue_name[256];
    sprintf(client_queue_name, "/mq_client_%u", getpid());
    strcpy(request.q_name, client_queue_name);

    // Send the request
    if (send_message(&request) < 0) { return -1; }

    // Receive the response
    if (receive_message(&response) < 0) { return -1; }

    // Close the queues
    mq_close(server_queue);
    mq_close(client_queue);
    mq_unlink(client_queue_name);

    //Return the response
    return response.res;
}

int get_value(int key, char *value1, int *N_value2, double *V_value2){
    // Obtiene el valor1 y el valor2 asociados a la clave key
    // Devuelve 0 en caso de éxito y -1 en caso de error, por ejemplo, si no existe un elemento con dicha clave o si se
    // produce un error de comunicaciones.

    // Handling errors in arguments

    // If any argument is NULL, we return -1
    if (value1 == NULL || V_value2 == NULL){
        return -1;
    }

    open_queues();

    // Create the request
    Request request;
    // Create the response
    Response response;

    // Fill the request
    request.op = 2;
    request.key = key;
    char client_queue_name[256];
    sprintf(client_queue_name, "/mq_client_%u", getpid());
    strcpy(request.q_name, client_queue_name);

    // Print 

    // Send the request
    if (send_message(&request) < 0) { return -1; }

    // Receive the response
    if (receive_message(&response) < 0) { return -1; }

    // Close the queues
    mq_close(server_queue);
    mq_close(client_queue);
    mq_unlink(client_queue_name);

    // Fill the output arguments
    strcpy(value1, response.value1);
    *N_value2 = response.N_value2;
    memcpy(V_value2, response.V_value2, response.N_value2 * sizeof(double));

    //Return the response
    return response.res;
}

int modify_value(int key, char *value1, int N_value2, double *V_value2){
    // Modifica los valores asociados a la clave key
    // Devuelve 0 en caso de éxito y -1 en caso de error, por ejemplo, si no existe un elemento con dicha clave o si se
    // produce un error en las comunicaciones. También se devolverá -1 si el valor N_value2 está fuera
    // de rango.

    // Handling errors in arguments

    // If any argument is NULL, we return -1
    if (value1 == NULL || V_value2 == NULL){
        return -1;
    }

    // if value1 has more tha 256 Bytes, we return -1
    if (strlen(value1) > 256){
        return -1;
    }
    
    // If N_value2 is not between 1 and 32, we return -1
    if (N_value2 < 1 || N_value2 > 32){
        return -1;
    }

    // If V_value2 has more than 32 elements, we return -1
    if (sizeof(V_value2) > 32){
        return -1;
    }

    open_queues();

    // Create the request
    Request request;
    // Create the response
    Response response;

    // Fill the request
    request.op = 3;
    request.key = key;
    strcpy(request.value1, value1);
    request.N_value2 = N_value2;
    memcpy(request.V_value2, V_value2, N_value2 * sizeof(double));
    char client_queue_name[256];
    sprintf(client_queue_name, "/mq_client_%u", getpid());
    strcpy(request.q_name, client_queue_name);

    // Send the request
    if (send_message(&request) < 0) { return -1; }

    // Receive the response
    if (receive_message(&response) < 0) { return -1; }

    // Close the queues
    mq_close(server_queue);
    mq_close(client_queue);
    mq_unlink(client_queue_name);

    //Return the response
    return response.res;
}

int delete_key(int key){
    // Borra el elemento cuya clave es key
    // Devuelve 0 en caso de éxito y -1 en caso de error. En caso de que la clave no exista
    // también se devuelve -1.

    open_queues();

    // Create the request
    Request request;
    // Create the response
    Response response;

    // Fill the request
    request.op = 4;
    request.key = key;
    char client_queue_name[256];
    sprintf(client_queue_name, "/mq_client_%u", getpid());
    strcpy(request.q_name, client_queue_name);

    // Send the request
    if (send_message(&request) < 0) { return -1; }

    // Receive the response
    if (receive_message(&response) < 0) { return -1; }

    // Close the queues
    mq_close(server_queue);
    mq_close(client_queue);
    mq_unlink(client_queue_name);

    //Return the response
    return response.res;
}

int exist(int key){
    // Determina si existe un elemento con clave key
    // Devuelve 1 en caso de que exista y 0 en caso de que no exista. En caso de error se
    // devuelve -1. Un error puede ocurrir en este caso por un problema en las comunicaciones.

    open_queues();

    // Create the request
    Request request;
    // Create the response
    Response response;

    // Fill the request
    request.op = 5;
    request.key = key;
    char client_queue_name[256];
    sprintf(client_queue_name, "/mq_client_%u", getpid());
    strcpy(request.q_name, client_queue_name);

    // Send the request
    if (send_message(&request) < 0) { return -1; }

    // Receive the response
    if (receive_message(&response) < 0) { return -1; }

    // Close the queues
    mq_close(server_queue);
    mq_close(client_queue);
    mq_unlink(client_queue_name);

    //Return the response
    return response.res;
}
