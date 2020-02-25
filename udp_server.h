#pragma once


#include "unp.h"
#include "My_procotol.h"

class My_UDP_server
{
public:
	My_UDP_server(int port, char* psw, char* filename);
	void build_socket(int, int, int);
	int bind_socket(short family, u_short port, char* str_addr, int addrlen);
	struct sockaddr_in get_sockaddr();
	void work_forever();
private:
	int sctfd;
	//uint32_t 
	struct sockaddr_in sock_addr;
	struct sockaddr_in client_addr;

	socklen_t client_addr_length;
	
	 char data_buffer[PACKSIZE];
	struct msg msg_buffer;
	 char psw[50];
	 char filename[512];


	SHA_CTX	sha1_c;
	unsigned char sha1_digest[20];

	int start_routine();
	bool varify_password();

	int send_pass_req();
	int send_pass_accept();
	int recv_join_req();
	int recv_pass_resp();
	int send_reject();
	int send_data();
	int send_terminate();
	//int send_data();
};



