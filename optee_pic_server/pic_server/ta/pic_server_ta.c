#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <tee_tcpsocket.h>
#include <tee_udpsocket.h>

#include <pic_server_ta.h>

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

static TEE_Result get_pic(uint32_t param_types, TEE_Param params[4])
{
	TEE_Result res;
	return TEE_SUCCESS;
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
        case TA_CMD_PIC_SERVER:
		return get_pic(param_types, params);
	default:
		return TEE_ERROR_NOT_SUPPORTED;
	}
}
