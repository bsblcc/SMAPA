#pragma once
#include "udp_client.h"







My_UDP_client::My_UDP_client(char* server_name, int port, char** psw, char* output_file)
{
	this->sctfd = -1;
	this->build_socket(AF_INET, SOCK_DGRAM, 0);
	this->bind_socket(AF_INET, port, "127.0.0.1", sizeof(struct sockaddr_in));  
	this->server_name = server_name;
	this->psw = psw;
	this->output_file = output_file;


}

My_UDP_client::My_UDP_client(int port)
{
	this->sctfd = -1;
	this->build_socket(AF_INET, SOCK_DGRAM, 0);
	this->bind_socket(AF_INET, port, "127.0.0.1", sizeof(struct sockaddr_in));  
}




void My_UDP_client::build_socket(int domain, int type, int protocol)
{
	this->sctfd = socket(domain, type, protocol);
	return;
}

int My_UDP_client::bind_socket(short family, u_short port, char* str_addr, int addrlen)
{

	if (this->sctfd == -1)
	{
		return -1;
	}

	bzero(&this->sock_addr,sizeof(this->sock_addr));
	this->sock_addr.sin_family = family;
	this->sock_addr.sin_addr.s_addr = inet_addr(str_addr);
	this->sock_addr.sin_port = htons(port);

	int result = bind(this->sctfd, (struct sockaddr*) &this->sock_addr, addrlen);
	return result;
}
int My_UDP_client::pack_and_send()
{
	int result;

	result = pack(data_buffer, &(this->msg_buffer));
	if (result != 0)
	{
		return result;
	}

	result = sendto(this->sctfd, data_buffer ,sizeof(data_buffer) , 0 ,(struct sockaddr *)&sock_addr,sizeof(sock_addr));
	if (result != PACKSIZE)
	{
		return -1;
	}

	return 0;

}

int My_UDP_client::recv_and_unpack()
{
	int result;
	memset(data_buffer, 0, sizeof(data_buffer));
	result = recv(this->sctfd, this->data_buffer, sizeof(data_buffer), 0);
	if (result != PACKSIZE)
	{
		return -1;
	}

	result = unpack(data_buffer, &(this->msg_buffer));
	if (result != 0)
	{
		return result;
	}



	return 0;
}

int My_UDP_client::send_join_req()
{
	cout << "start send JOIN_REQ\n";
	int result;

	memset(&(this->msg_buffer), 0 , sizeof(this->msg_buffer));

	this->msg_buffer.id = JOIN_REQ;
	this->msg_buffer.pyld_length = 0;

	result = this->pack_and_send();
	if (result != 0)
	{
		return result;
	}

	cout << "JOIN_REQ sent\n";

	return 0;
}
// 0 pass, 1 retry, 2 unpass, -1 exception
int My_UDP_client::recv_pass_result()
{
	cout << "start\n";

	int result;

	result = recv_and_unpack();
	if (result != 0)
	{
		return result;
	}

	if (this->msg_buffer.id == REJECT)
	{
		return 2;
	}

	if (this->msg_buffer.id == PASS_REQ)
	{
		return 1;
	}
	if (this->msg_buffer.id == PASS_ACCEPT)
	{
		return 0;
	}
	else
	{
		return -1;
	}



}

int My_UDP_client::send_pass_resp(int psw_count)
{
	cout << "start send PASS_RESP " << psw_count << endl;
	if (psw_count < 0 || psw_count > 3)
	{
		return -1;
	}
	int result;

	memset(&(this->msg_buffer), 0 , sizeof(this->msg_buffer));

	this->msg_buffer.id = PASS_RESP;


	this->msg_buffer.pyld_length = strlen(this->psw[psw_count]);
	//this->msg_buffer.payload = this->psw[psw_count];
	strcpy(this->msg_buffer.payload , this->psw[psw_count]);



	result = this->pack_and_send();
	if (result != 0)
	{
		return result;
	}



	cout << "PASS_RESP sent\n";
	return 0;
}

int My_UDP_client::recv_data()
{
	cout << "start recv DATA\n";
	int result;
	ofstream fout(this->output_file , ios::binary);

	SHA1_Init(&(this->sha1_c));	

	uint32_t packet_count;

	while(true)
	{
		result = this->recv_and_unpack();
		
		if (this->msg_buffer.id == TERMINATE)
		{
			cout << "DATA received\n";
			return 0;
		}
		if (this->msg_buffer.id != DATA)
		{
			return -1;
		}

		packet_count = ntohl(*((uint32_t*) this->msg_buffer.payload));
		cout << "data " << packet_count << " " << "received\n";

		// TODO :
		// check the packet count

		cout << "start update sha1" << endl;
		//SHA1_Update(&(this->sha1_c), this->msg_buffer.payload, this->msg_buffer.pyld_length);
		//cout << "length:" << msg_buffer.pyld_length << endl;

		SHA1_Update(&(this->sha1_c), this->msg_buffer.payload + 4, this->msg_buffer.pyld_length);



		fout.write((char*) this->msg_buffer.payload + 4, this->msg_buffer.pyld_length);


		//cout << "start update file buffer" << endl;
		//strcat(this->output_buffer, this->msg_buffer.payload);
		cout << "file wrote" << endl;
		
	}
}


int My_UDP_client::varify_data()
{
	cout << "start varify data\n";
	int result;



	SHA1_Final(this->sha1_digest, &(this->sha1_c));

	result = (memcmp(this->msg_buffer.payload, this->sha1_digest , this->msg_buffer.pyld_length) != 0);

	//char temp[20];
	//memcpy(temp, msg_buffer.payload, 20);

	//printf("digest:\na:%s  %d %d\nb:%s %d\n" , temp, strlen(temp), msg_buffer.pyld_length, (char*)sha1_digest, strlen((char*)sha1_digest));

	//printf("digest:\n%20s\n\n\n%20s\n" , this->msg_buffer.payload, this->sha1_digest);


	
	if (result != 0)
	{
		cout << "varification unpassed\n";
		cout << "ABORT" << endl;
		return 1;
	}
	//ofstream fout(this->output_file , ios::binary);

	//fout.write((char*) this->output_buffer, sizeof(this->output_buffer));

	cout << "varification passed\n";
	cout << "OK" << endl;
	return 0;
}


int My_UDP_client::try_connect()
{
	int result;

	
	result = this->send_join_req();
	if (result != 0)
	{
		return result;
	}

	result = this->recv_pass_result();
	if (result != 1)
	{
		return -1;
	}

	bool pass_flag = false;

	for (int req_count = 0; req_count < 3; req_count++)
	{
		cout << "try password " << req_count << endl;

		result = this->send_pass_resp(req_count);
		if (result != 0)
		{

			return -1;
		}


		result = this->recv_pass_result();


		if (result == 0)
		{
			cout << "password passed\n";
			pass_flag = true;
			break;
		}
		cout << "password retry\n";

	}

	if (!pass_flag)
	{
		cout << "password failed\n";
		return -2;
	}

	result = this->recv_data();
	if (result != 0)
	{
		return result;
	}

	result = this->varify_data();
	if (result != 0)
	{
		return result;
	}

	return 0;
}
	

int main(int argc, char** argv)
{

	char* psw[3];
	for (int i = 0; i < 3; i++)
	{
		psw[i] = argv[i + 3];
	}

	My_UDP_client test(argv[1], atoi(argv[2]), psw, argv[6]);
	test.try_connect();

	return 0;
}