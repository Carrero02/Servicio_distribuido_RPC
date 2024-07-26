/*
Code that implements the functions defined in claves.h
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../claves.h"

int init(){
    CLIENT *clnt;
	enum clnt_stat retval;
	int result;

    // try to get the environment variables
    char* IP_TUPLAS = getenv("IP_TUPLAS");
    if ( IP_TUPLAS == NULL) {
        perror("Error getting environment variables\n");
        return -1;
    }

    // Create the client
    clnt = clnt_create(IP_TUPLAS, CLAVES, CLAVESVER, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror(IP_TUPLAS);
        return -1;
    }
    retval = rpc_init_1(&result, clnt);
    if (retval != RPC_SUCCESS) {
        clnt_perror(clnt, "call failed");
    }
    // destroy the client
    clnt_destroy(clnt);

    return result;
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

    CLIENT *clnt;
    enum clnt_stat retval;
    int result;

    // try to get the environment variables
    char* IP_TUPLAS = getenv("IP_TUPLAS");
    if ( IP_TUPLAS == NULL) {
        perror("Error getting environment variables\n");
        return -1;
    }

    // Create the client
    clnt = clnt_create(IP_TUPLAS, CLAVES, CLAVESVER, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror(IP_TUPLAS);
        return -1;
    }

    // Create the args struct
    struct args rpc_set_value_1_arg;
    rpc_set_value_1_arg.key = key;
    rpc_set_value_1_arg.value1 = value1;
    rpc_set_value_1_arg.N_value2 = N_value2;
    for (int i = 0; i < N_value2; i++) {
        rpc_set_value_1_arg.V_value2[i] = V_value2[i];
    }

    // Call the remote procedure
    retval = rpc_set_value_1(rpc_set_value_1_arg, &result, clnt);
    if (retval != RPC_SUCCESS) {
        clnt_perror(clnt, "call failed");
    }

    clnt_destroy(clnt);
    return result;
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

    CLIENT *clnt;
    enum clnt_stat retval;
    struct get_value_result result;

    // try to get the environment variables
    char* IP_TUPLAS = getenv("IP_TUPLAS");
    if ( IP_TUPLAS == NULL) {
        perror("Error getting environment variables\n");
        return -1;
    }

    // Create the client
    clnt = clnt_create(IP_TUPLAS, CLAVES, CLAVESVER, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror(IP_TUPLAS);
        return -1;
    }

    // Allocate memory for result.value1
    result.value1 = malloc(256);
    if (result.value1 == NULL) {
        perror("Memory allocation failed");
        return -1;
    }
    
    // Call the remote procedure
    retval = rpc_get_value_1(key, &result, clnt);
    if (retval != RPC_SUCCESS) {
        clnt_perror(clnt, "call failed");
        free(result.value1);
        return -1;
    }

    // If the key does not exist, we return -1
    if (result.status == -1) {
        return -1;
    }

    // Check that result.value1 is not null before copying
    if (result.value1 != NULL) {
        strcpy(value1, result.value1);
    }

    // Copy the values to the arguments
    *N_value2 = result.N_value2;
    for (int i = 0; i < *N_value2; i++) {
        V_value2[i] = result.V_value2[i];
    }

    // Free the allocated memory
    free(result.value1);

    clnt_destroy(clnt);

    return result.status;
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

    // create the client
    CLIENT *clnt;
    enum clnt_stat retval;
    int result;

    // try to get the environment variables
    char* IP_TUPLAS = getenv("IP_TUPLAS");
    if ( IP_TUPLAS == NULL) {
        perror("Error getting environment variables\n");
        return -1;
    }

    // Create the client
    clnt = clnt_create(IP_TUPLAS, CLAVES, CLAVESVER, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror(IP_TUPLAS);
        return -1;
    }

    // Create the args struct
    struct args rpc_modify_value_1_arg;
    rpc_modify_value_1_arg.key = key;
    rpc_modify_value_1_arg.value1 = value1;
    rpc_modify_value_1_arg.N_value2 = N_value2;
    for (int i = 0; i < N_value2; i++) {
        rpc_modify_value_1_arg.V_value2[i] = V_value2[i];
    }

    // Call the remote procedure
    retval = rpc_modify_value_1(rpc_modify_value_1_arg, &result, clnt);
    if (retval != RPC_SUCCESS) {
        clnt_perror(clnt, "call failed");
    }
    clnt_destroy(clnt);
    return result;
}

int delete_key(int key){
    // Borra el elemento cuya clave es key
    // Devuelve 0 en caso de éxito y -1 en caso de error. En caso de que la clave no exista
    // también se devuelve -1.

    // create the client
    CLIENT *clnt;
    enum clnt_stat retval;
    int result;

    // try to get the environment variables
    char* IP_TUPLAS = getenv("IP_TUPLAS");
    if ( IP_TUPLAS == NULL) {
        perror("Error getting environment variables\n");
        return -1;
    }

    // Create the client
    clnt = clnt_create(IP_TUPLAS, CLAVES, CLAVESVER, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror(IP_TUPLAS);
        return -1;
    }

    // Call the remote procedure
    retval = rpc_delete_key_1(key, &result, clnt);
    if (retval != RPC_SUCCESS) {
        clnt_perror(clnt, "call failed");
    }

    clnt_destroy(clnt);
    return result;
}

int exist(int key){
    // Determina si existe un elemento con clave key
    // Devuelve 1 en caso de que exista y 0 en caso de que no exista. En caso de error se
    // devuelve -1. Un error puede ocurrir en este caso por un problema en las comunicaciones.

    // create the client
    CLIENT *clnt;
    enum clnt_stat retval;
    int result;

    // try to get the environment variables
    char* IP_TUPLAS = getenv("IP_TUPLAS");
    if ( IP_TUPLAS == NULL) {
        perror("Error getting environment variables\n");
        return -1;
    }

    // Create the client
    clnt = clnt_create(IP_TUPLAS, CLAVES, CLAVESVER, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror(IP_TUPLAS);
        return -1;
    }

    // Call the remote procedure
    retval = rpc_exist_1(key, &result, clnt);
    if (retval != RPC_SUCCESS) {
        clnt_perror(clnt, "call failed");
    }
    clnt_destroy(clnt);

    return result;
}
