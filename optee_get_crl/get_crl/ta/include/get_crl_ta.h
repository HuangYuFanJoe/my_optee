#ifndef TA_GET_CRL_H
#define TA_GET_CRL_H


#define TA_GET_CRL_UUID \
	{ 0x1af4f3ca, 0xb52c, 0x11ea, \
		{ 0xb3, 0xde, 0x02, 0x42, 0xac, 0x13, 0x00, 0x04} }

#define MAX_RECV_SIZE 1000

typedef struct {
	char *requester;
	uint32_t date;
} Request_Information;

/* The function IDs implemented in this TA */
#define TA_CMD_GET_SYSTEM_TIME 		0
#define TA_CMD_GET_REE_TIME		1
#define TA_CMD_GET_CRL			2

#endif /*TA_GET_CRL_H*/
