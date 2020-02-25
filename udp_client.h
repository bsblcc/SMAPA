
#pragma once



#include "unp.h"
#include "My_procotol.h"


class My_UDP_client
{
public:
	
	My_UDP_client(int port);
	My_UDP_client(char* server_name, int port, char** psw, char* output_file);
	void build_socket(int domain, int type, int protocol);
	int bind_socket(short family, u_short port, char* str_addr, int addrlen);

	int try_connect();
private:

	// method about data transportation
	int pack_and_send();
	int recv_and_unpack();


	struct sockaddr_in sock_addr;
	int sctfd;

	// method about procotol
	int send_join_req();
	int recv_pass_result();
	int send_pass_resp(int psw_count);
	int recv_data();
	int varify_data();

	struct msg msg_buffer;
	 char data_buffer[PACKSIZE];
	

	
	SHA_CTX	sha1_c;
	unsigned char sha1_digest[20];


	 char* server_name;
	 char** psw;
	 char* output_file;

	 //char output_buffer[12344];
};
