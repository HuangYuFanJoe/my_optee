#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <tee_tcpsocket.h>
#include <tee_udpsocket.h>

#include <pic_server_ta.h>

struct aes_cipher {
	uint32_t algo;			/* AES flavour */
	uint32_t mode;			/* Encode or decode */
	uint32_t key_size;		/* AES key size in byte */
	TEE_OperationHandle op_handle;	/* AES ciphering operation */
	TEE_ObjectHandle key_handle;	/* transient object to load the key */
};

static struct aes_cipher *sess;
static TEE_Attribute attr;
static int key_size;
char *AES_key, *Nonce;

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
		void __maybe_unused **session)
{
	TEE_Result res;
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	IMSG("Open session entry point has been called.\n");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	if(params[0].value.a != 0)
		key_size = params[0].value.a;
	else
		goto out;
	

	sess = TEE_Malloc(sizeof(*sess), 0);
	if (!sess)
		return TEE_ERROR_OUT_OF_MEMORY;

	sess->key_handle = TEE_HANDLE_NULL;
	sess->op_handle  = TEE_HANDLE_NULL;
	sess->algo       = TEE_ALG_AES_GCM;
	sess->key_size   = (key_size / 8);
	sess->mode	 = TA_AES_MODE_ENCODE;

	/* Allocate operation: AES/GCM, mode and size from params */
	res = TEE_AllocateOperation(&sess->op_handle,
				    sess->algo,
				    sess->mode,
				    sess->key_size * 8);
	if (res != TEE_SUCCESS) {
		EMSG("Failed to allocate operation");
		sess->op_handle = TEE_HANDLE_NULL;
		return res;
	}

	/* Allocate transient object according to target key size */
	res = TEE_AllocateTransientObject(TEE_TYPE_AES,
					  sess->key_size * 8,
					  &sess->key_handle);
	if (res != TEE_SUCCESS) {
		EMSG("Failed to allocate transient object");
		sess->key_handle = TEE_HANDLE_NULL;
		return res;
	}

	AES_key = TEE_Malloc(sess->key_size, 0);
	if (!AES_key) {
		res = TEE_ERROR_OUT_OF_MEMORY;
		return res;
	}
	memset(AES_key, 0xa5, key_size / 8); /* Load some dummy value */

	TEE_InitRefAttribute(&attr, TEE_ATTR_SECRET_VALUE, AES_key, sess->key_size);

	res = TEE_PopulateTransientObject(sess->key_handle, &attr, 1);
	if (res != TEE_SUCCESS) {
		EMSG("TEE_PopulateTransientObject failed, %x", res);
		return res;
	}

	res = TEE_SetOperationKey(sess->op_handle, sess->key_handle);
	if (res != TEE_SUCCESS) {
		EMSG("TEE_SetOperationKey failed %x", res);
		return res;
	}

	Nonce = TEE_Malloc(AES_BLOCK_SIZE, 0);
	if (!Nonce) {
		res = TEE_ERROR_OUT_OF_MEMORY;
		return res;
	}
	memset(Nonce, 0x00, AES_BLOCK_SIZE); /* Load some dummy value */

	TEE_AEInit(sess->op_handle, Nonce, AES_BLOCK_SIZE, 128, 0, 0);

	*session = (void *)sess;

out:
	IMSG("TA Open session sucessfully\n");

	return TEE_SUCCESS;
}

/*
 * Called when a session is closed.
 */
void TA_CloseSessionEntryPoint(void __maybe_unused *sess_ctx)
{
	(void)&sess_ctx;
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

void strcat(char* dest, const char* src)
{
	while (*dest)
		dest++;
	while ((*dest++ = *src++) != '\0')
		;
}

static TEE_Result aes_encrypt(uint32_t param_types, TEE_Param params[4])
{
	TEE_Result res;
	char *inbuf = NULL, *outbuf = NULL, *srcData = NULL, *destData = NULL, *tag = NULL;
	size_t inbuf_len = 0, outbuf_len = 0, chunksize = 128, tag_len = 128;

	const uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
						TEE_PARAM_TYPE_MEMREF_INOUT,
						TEE_PARAM_TYPE_NONE,
						TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	inbuf = (char *)params[0].memref.buffer;
	inbuf_len = params[0].memref.size;
	outbuf = (char *)params[1].memref.buffer;
	outbuf_len = params[1].memref.size;

	srcData = (char *)TEE_Malloc(chunksize + 1, TEE_MALLOC_FILL_ZERO);
	if(!srcData)
		return TEE_ERROR_OUT_OF_MEMORY;

	destData = (char *)TEE_Malloc(chunksize + 1, TEE_MALLOC_FILL_ZERO);
	if(!destData)
		return TEE_ERROR_OUT_OF_MEMORY;

	tag = (char *)TEE_Malloc(chunksize + 1, TEE_MALLOC_FILL_ZERO);
	if(!tag)
		return TEE_ERROR_OUT_OF_MEMORY;

	while(inbuf_len > chunksize){
		strncpy(srcData, inbuf, chunksize);
		TEE_AEUpdate(sess->op_handle, srcData, chunksize, destData, &chunksize);
		strcat(outbuf, destData);
		inbuf = inbuf + chunksize; // shift left
		inbuf_len -= chunksize;
	}
	strncpy(srcData, inbuf, inbuf_len);
	TEE_AEEncryptFinal(sess->op_handle, srcData, inbuf_len, destData, &inbuf_len, tag, &tag_len);
	strcat(outbuf, destData);
	TEE_Free(srcData);
	TEE_Free(destData);

	params[1].memref.size = strlen(outbuf);

	return res;	
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
	case TA_ACIPHER_CMD_ENCRYPT:
		return aes_encrypt(param_types, params);
	default:
		return TEE_ERROR_NOT_SUPPORTED;
	}
}
