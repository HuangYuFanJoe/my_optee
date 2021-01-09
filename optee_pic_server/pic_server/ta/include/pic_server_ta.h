#ifndef TA_PIC_SERVER_H
#define TA_PIC_SERVER_H

#define TA_PIC_SERVER_UUID \
	{ 0x376812ac, 0xcdf9, 0x47d8, \
		{ 0x90, 0x05, 0xab, 0x70, 0x4b, 0xfa, 0xa9, 0x1a} }

#define MAX_RECV_SIZE 			10*1024*1024
#define MAX_REQUESTER_SIZE		20
#define MAX_LINE				4096
#define LISTENPORT				8787
#define SERVERPORT				8080
#define MAX_sTIME				30
#define MAX_FILEPATH_SIZE		50
#define KEY_SIZE				256
#define AES_BLOCK_SIZE			16

typedef struct {
	char requester[MAX_REQUESTER_SIZE];
	char *date;
} Receive_Information;

/* The function IDs implemented in this TA */
#define TA_CMD_GET_SYSTEM_TIME 		0
#define TA_CMD_GET_REE_TIME			1
#define TA_CMD_PIC_SERVER			2
#define TA_ACIPHER_CMD_ENCRYPT		3

#define TA_AES_MODE_ENCODE			1
#define TA_AES_MODE_DECODE			0

#endif /*TA_PIC_SERVER_H*/
