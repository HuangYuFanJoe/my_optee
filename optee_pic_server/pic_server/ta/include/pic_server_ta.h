#ifndef TA_PIC_SERVER_H
#define TA_PIC_SERVER_H


#define TA_PIC_SERVER_UUID \
	{ 0x376812ac, 0xcdf9, 0x47d8, \
		{ 0x90, 0x05, 0xab, 0x70, 0x4b, 0xfa, 0xa9, 0x1a} }

#define TA_MAX_RECV_SIZE 5000
#define MAX_REQUESTER_SIZE 20
#define MAX_LINE 4096
#define LISTENPORT 8787
#define SERVERPORT 8080
#define MAX_sTIME 30

typedef struct {
	char requester[MAX_REQUESTER_SIZE];
	char data[MAX_LINE];
	char *date;
} Receive_Information;

/* The function IDs implemented in this TA */
#define TA_CMD_GET_SYSTEM_TIME 		0
#define TA_CMD_GET_REE_TIME			1
#define TA_CMD_PIC_SERVER			2

#endif /*TA_PIC_SERVER_H*/
