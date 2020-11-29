#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <tee_tcpsocket.h>
#include <tee_udpsocket.h>

#include <get_crl_ta.h>

/*
 * Called when the instance of the TA is created.
 */
TEE_Result TA_CreateEntryPoint(void)
{
	IMSG("Create entry point has been called.\n");

	return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void)
{
	IMSG("Destroy entry point has been called.\n");
}

/*
 * Called when a new session is opened to the TA.
 */
TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
		TEE_Param __maybe_unused params[4],
		void __maybe_unused **sess_ctx)
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	IMSG("Open session entry point has been called.\n");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Unused parameters */
	(void)&params;
	(void)&sess_ctx;

	IMSG("TA session is opening!!!\n");

	return TEE_SUCCESS;
}

/*
 * Called when a session is closed.
 */
void TA_CloseSessionEntryPoint(void __maybe_unused *sess_ctx)
{
	(void)&sess_ctx; /* Unused parameter */
	IMSG("Goodbye!\n");
}

static TEE_Result tcp_connect(TEE_tcpSocket_Setup *setup,
			      TEE_iSocketHandle *ctx)
{
	TEE_Result res;
	uint32_t proto_error;

	setup->ipVersion   = TEE_IP_VERSION_DC;
	setup->server_addr = "140.115.52.122";
	setup->server_port = "8080";

	res = TEE_tcpSocket->open(ctx, setup, &proto_error);
	if (res != TEE_SUCCESS) {
		EMSG("TCP_Socket open() failed. Return code: %u"
		     ", protocol error: %u", res, proto_error);
		return res;
	}

	return TEE_SUCCESS;
}

static TEE_Result udp_connect(TEE_udpSocket_Setup *setup,
			      TEE_iSocketHandle *ctx)
{
	TEE_Result res;
	uint32_t proto_error;

	setup->ipVersion   = TEE_IP_VERSION_DC;
	setup->server_addr = "140.115.52.122";
	setup->server_port = "8080";

	res = TEE_udpSocket->open(ctx, setup, &proto_error);
	if (res != TEE_SUCCESS) {
		EMSG("UDP_Socket open() failed. Return code: %u"
		     ", protocol error: %u", res, proto_error);
		return res;
	}

	return TEE_SUCCESS;
}

static char *prepare_message(uint32_t param_types, TEE_Param params[4], int *message_size)
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;
    
	TEE_Time ree_time;
	TEE_GetREETime(&ree_time);

	Request_Information *RI;
	RI = params[0].memref.buffer;
	RI->date = ree_time.seconds;
	RI->requester = "Yufan";
	

	*message_size = snprintf(NULL, 0, 
			"Requester=%s , date=%d", RI->requester, RI->date);

	IMSG("Request Information message size = %d\n", *message_size);
	
	char *message_buffer = NULL;
	message_buffer = (char *)TEE_Malloc(*message_size + 1, TEE_MALLOC_FILL_ZERO);
	if (message_buffer == NULL)
		return message_buffer;

	snprintf(message_buffer, *message_size + 1,
		"Requester=%s , date=%d", RI->requester, RI->date);

	return message_buffer;
}

static TEE_Result socket_client(uint32_t param_types, TEE_Param params[4])
{
	TEE_Result res;

	// for Relation server
	TEE_iSocket *socket_Relation = NULL;
	TEE_iSocketHandle socketCtx_Relation;
	TEE_tcpSocket_Setup tcpSetup_Relation;
	TEE_udpSocket_Setup udpSetup_Relation;

	char *message, *recv_message;
	int message_size = 0, recv_message_size = MAX_RECV_SIZE;

	message = prepare_message(param_types, params, &message_size);
	if (message == NULL) {
		return TEE_ERROR_OUT_OF_MEMORY;
	}

	IMSG("Request message:\n%s", message);

	IMSG("Send request to Relation Server...\n");
	res = tcp_connect(&tcpSetup_Relation, &socketCtx_Relation);
	//res = udp_connect(&udpSetup_Relation, &socketCtx_Relation, s_args_Relation);

	if (res != TEE_SUCCESS) {
        IMSG("TCP/UDP connect failed!!\n");
		return TEE_ERROR_BAD_PARAMETERS;
	}

	res = socket_Relation->send(socketCtx_Relation, message, &message_size, TEE_TIMEOUT_INFINITE);
	if (res != TEE_SUCCESS) {
		EMSG("Relation server socket send() failed. Error code: %#0" PRIX32, res);
		return res;
	}

	IMSG("Send request to Relation server successfully!!!\n");
    

        res = socket_Relation->recv(socketCtx_Relation, recv_message, &recv_message_size, TEE_TIMEOUT_INFINITE);
	if (res != TEE_SUCCESS) {
		EMSG("Relation server socket recv() failed. Error code: %#0" PRIX32, res);
		return res;
	}

	IMSG("Receive CRL successfully!!!\n");
        IMSG("Receive message: %s\n", recv_message);
    
	socket_Relation->close(socketCtx_Relation);
	
	return res;
}

static TEE_Result get_system_time(uint32_t param_types, TEE_Param params[4])
{
	TEE_Time system_time;
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_OUTPUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	IMSG("TA get REE time!!!\n");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;


	/*
	 * Call the TEE internal time API to get the REE time.
	 */
	IMSG("Call the TEE internal time API to get the REE time.\n");
	TEE_GetSystemTime(&system_time);

	params[0].value.a = system_time.seconds;
	params[0].value.b = system_time.millis;

	return TEE_SUCCESS;
}

static TEE_Result get_ree_time(uint32_t param_types, TEE_Param params[4])
{
	TEE_Time ree_time;
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_OUTPUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	IMSG("TA get REE time!!!\n");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;


	/*
	 * Call the TEE internal time API to get the REE time.
	 */
	IMSG("Call the TEE internal time API to get the REE time.\n");
	TEE_GetREETime(&ree_time);

	params[0].value.a = ree_time.seconds;
	params[0].value.b = ree_time.millis;

	return TEE_SUCCESS;
}


/*
 * Called when a TA is invoked.
 */
TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
				      uint32_t cmd_id,
				      uint32_t param_types, TEE_Param params[4])
{

	switch (cmd_id) {
	case TA_CMD_GET_SYSTEM_TIME:
		return get_system_time(param_types, params);
	case TA_CMD_GET_REE_TIME:
		return get_ree_time(param_types, params);
        case TA_CMD_GET_CRL:
		return socket_client(param_types, params);
	default:
		return TEE_ERROR_NOT_SUPPORTED;
	}
}
