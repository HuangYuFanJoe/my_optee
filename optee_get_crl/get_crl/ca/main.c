#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>

/* To the the UUID (found the the TA's h-file(s)) */
#include <get_crl_ta.h>

void get_TEE_time(TEEC_Result *result, TEEC_Session *session,
		  TEEC_Operation *operation, uint32_t *err_origin)
{
	uint32_t seconds, millis;

	/*
	 * Prepare the argument
	 */
	operation->paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT, TEEC_NONE,
						TEEC_NONE, TEEC_NONE);

	/*
	 * TA_OBFUSCATION_CMD_GET_SYSTEM_TIME
	 */
	printf("Invoke TA to get the TEE system time.\n");
	*result = TEEC_InvokeCommand(session, TA_CMD_GET_SYSTEM_TIME,
				     operation, err_origin);
	if (*result != TEEC_SUCCESS) {
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
			*result, *err_origin);
	}
	printf("TA get the TEE system time successfully.\n");

	seconds = operation->params[0].value.a;
	millis = operation->params[0].value.b;

	printf("The TEE system time:\n");
	printf("Seconds: %d\n Millis: %d\n", seconds, millis);
}

void get_REE_time(TEEC_Result *result, TEEC_Session *session,
		  TEEC_Operation *operation, uint32_t *err_origin)
{
	uint32_t seconds, millis;

	/*
	 * Prepare the argument
	 */
	operation->paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT, TEEC_NONE,
						TEEC_NONE, TEEC_NONE);

	/*
	 * TA_OBFUSCATION_CMD_GET_REE_TIME
	 */
	printf("Invoke TA to get the REE system time.\n");
	*result = TEEC_InvokeCommand(session, TA_CMD_GET_REE_TIME,
				     operation, err_origin);
	if (*result != TEEC_SUCCESS) {
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
			*result, *err_origin);
	}
	printf("TA get the REE system time successfully.\n");

	seconds = operation->params[0].value.a;
	millis = operation->params[0].value.b;

	printf("The REE system time:\n");
	printf("Seconds: %d\n Millis: %d\n", seconds, millis);
}

Request_Information *get_request_info(TEEC_Result *result,
					      TEEC_Session *session,
					      TEEC_Operation *operation,
					      uint32_t *err_origin)
{
	/*
	 * Prepare the argument
	 */
	Request_Information *RI;
	RI = (Request_Information *) malloc(sizeof(Request_Information));
	if (!RI) {
		return NULL;
	}
	memset(RI, 0, sizeof(Request_Information));

	operation->paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_OUTPUT,
						 TEEC_NONE,
						 TEEC_NONE,
						 TEEC_NONE);

	operation->params[0].tmpref.buffer = RI;
	operation->params[0].tmpref.size = sizeof(Request_Information);
	/*
	 * TA_CMD_GET_CRL
	 */
	//printf("Invoke TA to get the CRL from server\n");
	*result = TEEC_InvokeCommand(session, TA_CMD_GET_CRL,
	 			     operation, err_origin);
	if (*result != TEEC_SUCCESS) {
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
			*result, *err_origin);
	}
	
	printf("Get the request information successfully.\n");

	printf("Requester : %s\n",  RI->requester);
	printf("Date: %d\n", RI->date);

	return RI;
}

int main(int argc, char *argv[])
{
	int choice;
	if (argc < 2) {
		printf("Usage: %s get_time 0|1|2 (0 -> system time | 1 -> REE time | 2 -> get CRL) \n", argv[0]);
		exit(EXIT_FAILURE);
	}
 	choice = atoi(argv[1]); 
	if (choice != 0 && choice != 1 && choice != 2) {
		printf("Choice must be 0、1 or 2 only!!!\n");
		exit(EXIT_FAILURE);
	}
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = TA_GET_CRL_UUID;
	uint32_t err_origin;
    Request_Information *RI = NULL;

	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

	/*
	 * Open a session to the "hello world" TA, the TA will print "hello
	 * world!" in the log when the session is created.
	 */
	res = TEEC_OpenSession(&ctx, &sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
			res, err_origin);

	/*
	 * Execute a function in the TA by invoking it, in this case
	 * we're incrementing a number.
	 *
	 * The value of command ID part and how the parameters are
	 * interpreted is part of the interface provided by the TA.
	 */

	/* Clear the TEEC_Operation struct */
	memset(&op, 0, sizeof(op));
    
	
	switch(choice){
    case 2 :
        RI = get_request_info(&res, &sess, &op, &err_origin);
        break;
	case 1 :
		get_TEE_time(&res, &sess, &op, &err_origin);
		break;
	case 0 :
		get_REE_time(&res, &sess, &op, &err_origin);
		break;
	default:
		break;

	}
	/*
	 * We're done with the TA, close the session and
	 * destroy the context.
	 *
	 * The TA will print "Goodbye!" in the log when the
	 * session is closed.
	 */

	TEEC_CloseSession(&sess);
	TEEC_FinalizeContext(&ctx);
	return 0;
}
