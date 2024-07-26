/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "claves.h"
#include "funciones_servidor/funciones_servidor.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

bool_t
rpc_init_1_svc(int *result, struct svc_req *rqstp)
{
	bool_t retval;
	//set retval
	retval = TRUE;
	// lock
	pthread_mutex_lock(&mutex);
	// call the init function
	*result = init();
	// unlock
	pthread_mutex_unlock(&mutex);

	return retval;
}

bool_t
rpc_set_value_1_svc(struct args arg, int *result,  struct svc_req *rqstp)
{
	bool_t retval;

	//set retval
	retval = TRUE;
	// lock
	pthread_mutex_lock(&mutex);
	// call the set_value function
	*result = set_value(arg.key, arg.value1, arg.N_value2, arg.V_value2);
	// unlock
	pthread_mutex_unlock(&mutex);

	return retval;
}

bool_t
rpc_get_value_1_svc(int key, get_value_result *result,  struct svc_req *rqstp)
{
	bool_t retval;

	result->value1 = malloc(256);

	retval = TRUE;
	// lock
	pthread_mutex_lock(&mutex);
	// call the get_value function
	result->status = get_value(key, result->value1, &result->N_value2, result->V_value2);
	// unlock
	pthread_mutex_unlock(&mutex);
	return retval;
}

bool_t
rpc_modify_value_1_svc(struct args arg, int *result,  struct svc_req *rqstp)
{
	bool_t retval;

	//set retval
	retval = TRUE;
	// lock
	pthread_mutex_lock(&mutex);
	// call the modify_value function
	*result = modify_value(arg.key, arg.value1, arg.N_value2, arg.V_value2);
	// unlock
	pthread_mutex_unlock(&mutex);

	return retval;
}

bool_t
rpc_delete_key_1_svc(int key, int *result,  struct svc_req *rqstp)
{
	bool_t retval;
	// lock
	pthread_mutex_lock(&mutex);
	// call the delete_key function
	*result = delete_key(key);
	// unlock
	pthread_mutex_unlock(&mutex);

	return retval;
}

bool_t
rpc_exist_1_svc(int key, int *result,  struct svc_req *rqstp)
{
	bool_t retval;
	// lock
	pthread_mutex_lock(&mutex);
	// call the exist function
	*result = exist(key);
	// unlock
	pthread_mutex_unlock(&mutex);

	return retval;
}

int
claves_1_freeresult (SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result)
{
	xdr_free (xdr_result, result);

	/*
	 * Insert additional freeing code here, if needed
	 */

	return 1;
}
