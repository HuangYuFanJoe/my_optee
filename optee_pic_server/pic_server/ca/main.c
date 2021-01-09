#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <inttypes.h>

/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>

/* To the the UUID (found the the TA's h-file(s)) */
#include <pic_server_ta.h>

struct Thread_arguments {
	int connfd;
	int count;
	char data[MAX_LINE + 1];
	TEEC_Result *result;
	TEEC_Session *session;
	TEEC_Operation *operation;
	uint32_t *err_origin;
};

void get_TEE_time()
{
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = TA_PIC_SERVER_UUID;
	uint32_t err_origin;
	uint32_t seconds, millis;

	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

	memset(&op, 0, sizeof(op));

	/* Open a session to the "picture server" TA */
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
					 TEEC_NONE, TEEC_NONE, TEEC_NONE);
	op.params[0].value.a = 0;

	res = TEEC_OpenSession(&ctx, &sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, &op, &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
			res, err_origin);

	memset(&op, 0, sizeof(op));

	/*
	 * Prepare the argument
	 */
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT, TEEC_NONE,
						TEEC_NONE, TEEC_NONE);

	/*
	 * TA_OBFUSCATION_CMD_GET_SYSTEM_TIME
	 */
	printf("Invoke TA to get the TEE system time.\n");
	res = TEEC_InvokeCommand(&sess, TA_CMD_GET_SYSTEM_TIME,
				     &op, &err_origin);
	if (res != TEEC_SUCCESS) {
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
			res, err_origin);
	}
	printf("TA get the TEE system time successfully.\n");

	seconds = op.params[0].value.a;
	millis = op.params[0].value.b;

	printf("The TEE system time:\n");
	printf("Seconds: %d\n Millis: %d\n", seconds, millis);
	
	TEEC_CloseSession(&sess);
	TEEC_FinalizeContext(&ctx);
}

void get_REE_time()
{
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = TA_PIC_SERVER_UUID;
	uint32_t err_origin;
	uint32_t seconds, millis;

	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

	memset(&op, 0, sizeof(op));

	/* Open a session to the "picture server" TA */
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
					 TEEC_NONE, TEEC_NONE, TEEC_NONE);
	op.params[0].value.a = 0;

	res = TEEC_OpenSession(&ctx, &sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, &op, &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
			res, err_origin);

	memset(&op, 0, sizeof(op));

	/*
	 * Prepare the argument
	 */
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT, TEEC_NONE,
						TEEC_NONE, TEEC_NONE);

	/*
	 * TA_OBFUSCATION_CMD_GET_REE_TIME
	 */
	printf("Invoke TA to get the REE system time.\n");
	res = TEEC_InvokeCommand(&sess, TA_CMD_GET_REE_TIME,
				     &op, &err_origin);
	if (res != TEEC_SUCCESS) {
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
			res, err_origin);
	}
	printf("TA get the REE system time successfully.\n");

	seconds = op.params[0].value.a;
	millis = op.params[0].value.b;

	printf("The REE system time:\n");
	printf("Seconds: %d\n Millis: %d\n", seconds, millis);

	TEEC_CloseSession(&sess);
	TEEC_FinalizeContext(&ctx);
}

char* aes_encrypt(char* picture)
{
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = TA_PIC_SERVER_UUID;
	uint32_t err_origin;

	char encrypt_picture[strlen(picture)];

	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

	memset(&op, 0, sizeof(op));

	/*
	 * Open a session to the "picture server" TA
	 * Send the AES information first
	 */

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
					 TEEC_NONE, TEEC_NONE, TEEC_NONE);
	op.params[0].value.a = KEY_SIZE;

	res = TEEC_OpenSession(&ctx, &sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, &op, &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
			res, err_origin);

	memset(&op, 0, sizeof(op));

	/* Prepare the argument */
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
					 TEEC_MEMREF_TEMP_INOUT,
					 TEEC_NONE, TEEC_NONE);

	op.params[0].tmpref.buffer = picture;
	op.params[0].tmpref.size = strlen(picture);

	memset(encrypt_picture, '\0', strlen(picture));
	op.params[1].tmpref.buffer = encrypt_picture;
	op.params[1].tmpref.size = strlen(picture);

	res = TEEC_InvokeCommand(&sess, TA_ACIPHER_CMD_ENCRYPT, &op, &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x", res, err_origin);

	TEEC_CloseSession(&sess);
	TEEC_FinalizeContext(&ctx);

	return (char *)op.params[1].tmpref.buffer;
}

void *serverThread(void *v_args){
	struct Thread_arguments *args = (struct Thread_arguments *)v_args;

	char count[2] = {};
	sprintf(count, "%d", args->count);
	char filepath[MAX_FILEPATH_SIZE] = "/data/image";
	strcat(filepath, count);
	strcat(filepath, ".png");

	ssize_t n, total = 0;
	char *picture = malloc(MAX_RECV_SIZE * sizeof(char));
	memset(picture , '\0', MAX_RECV_SIZE * sizeof(char));
	//FILE *fp = fopen(filepath, "wb");
	while((n = recv(args->connfd, args->data, MAX_LINE, 0)) > 0){
		strcat(picture, args->data);
		/*if(fwrite(args->data, sizeof(char), n, fp) != n){
			perror("Write File Error");
			exit(1);
		}*/
		total += n;
		memset(args->data, '\0', MAX_LINE);
	}
	//fclose(fp);
	close(args->connfd);

	picture = aes_encrypt(picture);
	FILE *fp = fopen(filepath, "wb");
	if(fwrite(picture, sizeof(char), strlen(picture), fp) != n){
			perror("Write File Error");
			exit(1);
	}
	fclose(fp);
	free(picture);
	pthread_exit(NULL);
}

Receive_Information *get_pic()
{
	/*
	 * Prepare the argument
	 */
	Receive_Information *RI;
	RI = (Receive_Information *) malloc(sizeof(Receive_Information));
	if (!RI) {
		return NULL;
	}
	memset(RI, 0, sizeof(Receive_Information));
	
	struct Thread_arguments args;

	/* Socket connect */

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) 
	{
		perror("Can't allocate sockfd");
		exit(1);
	}

	struct sockaddr_in clientaddr, serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	
	bind(sockfd, (const struct sockaddr *) &serveraddr, sizeof(serveraddr));
	listen(sockfd, LISTENPORT);
	
	socklen_t addrlen = sizeof(clientaddr);
	int count = 1;
	while(true){  // receive data
		int connfd = accept(sockfd, (struct sockaddr *) &clientaddr, &addrlen);
		//recv(connfd, RI->requester, MAX_REQUESTER_SIZE, 0);
		//printf("Requester : %s\n",  RI->requester);
		time_t mytime = time(NULL);
		RI->date = ctime(&mytime);
		printf("Date: %s\n", RI->date);
		args.connfd = connfd;
		args.count = count;
        pthread_t t;
        pthread_create(&t, NULL, &serverThread, (void *)&args);
		count = (count % 10) + 1;
	}

	/*op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_OUTPUT,
					 TEEC_NONE,
					 TEEC_NONE,
					 TEEC_NONE);
	
	// TA_CMD_PIC_SERVER

	printf("Invoke TA to send the picture to TEE\n");
	res = TEEC_InvokeCommand(&sess, TA_CMD_PIC_SERVER,
	 			     &op, &err_origin);
	if (res != TEEC_SUCCESS) {
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
			res, err_origin);
	}
	op.params[0].tmpref.buffer = RI;
	op.params[0].tmpref.size = sizeof(Receive_Information);
	printf("Send the picture to TEE successfully.\n");*/

	return RI;
}


int main(int argc, char *argv[])
{
	int choice;
	if (argc < 2) {
		printf("Usage: %s get_time 0|1|2 (0 -> system time | 1 -> REE time | 2 -> run pic server) \n", argv[0]);
		exit(EXIT_FAILURE);
	}
 	choice = atoi(argv[1]); 
	if (choice != 0 && choice != 1 && choice != 2) {
		printf("Choice must be 0、1 or 2 only!!!\n");
		exit(EXIT_FAILURE);
	}

	Receive_Information *RI = NULL;
    
	switch(choice){
		case 2 :
		    RI = get_pic();
		    break;
		case 1 :
		    get_TEE_time();
		    break;
		case 0 :
		    get_REE_time();
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

	return 0;
}
