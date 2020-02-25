#pragma once

#include "unp.h"

#define JOIN_REQ 1
#define PASS_REQ 2
#define PASS_RESP 3
#define PASS_ACCEPT 4
#define DATA 5
#define TERMINATE 6
#define REJECT 7

#define PACKSIZE 1010
struct msg
{
	uint16_t id;
	uint32_t pyld_length;
	char payload[1004];

};
bool isID(int id)
{
	return id > 0 && id <= 7;
}

bool id_has_payload(int id)
{
	return id == PASS_RESP || id == DATA || id == TERMINATE;
}

bool isLegal(struct msg* msg)
{
	if (!isID(msg->id))
	{
		return false;
	}
	if (msg->pyld_length < 0)
	{
		return false;
	}
	if (!id_has_payload(msg->id) && msg->pyld_length != 0)
	{
		return false;
	}
	return true;
}

int unpack(char* data, struct msg* msg)
{
	// so actually this ntohs families can deal with architectures in both
	// big and little endian.
	memset(msg, 0, sizeof(msg));
	msg->id = ntohs(*((uint16_t*) data));
	msg->pyld_length = ntohl(*((uint32_t*) (data + 2)));

	//msg->payload = data + 6;
	// Im not sure if this "data" memory will be destructed during this.
	memcpy(msg->payload, data + 6, sizeof(msg->payload));
	return isLegal(msg) ? 0 : -1;
}

int pack(char* data, struct msg* msg)
{

	if (!isLegal(msg))
	{
		return -1;
	}
	memset(data, 0, sizeof(data));
	*((uint16_t*) data) = htons(msg->id);
	*((uint32_t*) (data + 2)) = htonl(msg->pyld_length);

	
	memcpy(data + 6, msg->payload, sizeof(msg->payload));

	return 0;
}