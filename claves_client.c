/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "claves.h"


void
claves_1(char *host)
{
	CLIENT *clnt;
	enum clnt_stat retval_1;
	int result_1;
	enum clnt_stat retval_2;
	int result_2;
	struct args rpc_set_value_1_arg;
	enum clnt_stat retval_3;
	get_value_result result_3;
	int rpc_get_value_1_key;
	enum clnt_stat retval_4;
	int result_4;
	struct args rpc_modify_value_1_arg;
	enum clnt_stat retval_5;
	int result_5;
	int rpc_delete_key_1_key;
	enum clnt_stat retval_6;
	int result_6;
	int rpc_exist_1_key;

#ifndef	DEBUG
	clnt = clnt_create (host, CLAVES, CLAVESVER, "udp");
	if (clnt == NULL) {
		clnt_pcreateerror (host);
		exit (1);
	}
#endif	/* DEBUG */

	retval_1 = rpc_init_1(&result_1, clnt);
	if (retval_1 != RPC_SUCCESS) {
		clnt_perror (clnt, "call failed");
	}
	retval_2 = rpc_set_value_1(rpc_set_value_1_arg, &result_2, clnt);
	if (retval_2 != RPC_SUCCESS) {
		clnt_perror (clnt, "call failed");
	}
	retval_3 = rpc_get_value_1(rpc_get_value_1_key, &result_3, clnt);
	if (retval_3 != RPC_SUCCESS) {
		clnt_perror (clnt, "call failed");
	}
	retval_4 = rpc_modify_value_1(rpc_modify_value_1_arg, &result_4, clnt);
	if (retval_4 != RPC_SUCCESS) {
		clnt_perror (clnt, "call failed");
	}
	retval_5 = rpc_delete_key_1(rpc_delete_key_1_key, &result_5, clnt);
	if (retval_5 != RPC_SUCCESS) {
		clnt_perror (clnt, "call failed");
	}
	retval_6 = rpc_exist_1(rpc_exist_1_key, &result_6, clnt);
	if (retval_6 != RPC_SUCCESS) {
		clnt_perror (clnt, "call failed");
	}
#ifndef	DEBUG
	clnt_destroy (clnt);
#endif	 /* DEBUG */
}


int
main (int argc, char *argv[])
{
	char *host;

	if (argc < 2) {
		printf ("usage: %s server_host\n", argv[0]);
		exit (1);
	}
	host = argv[1];
	claves_1 (host);
exit (0);
}
