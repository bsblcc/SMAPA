
#include "udp_server.h"










My_UDP_server::My_UDP_server(int port, char* psw, char* filename)
{
	this->sctfd = -1;
	this->build_socket(AF_INET, SOCK_DGRAM, 0);
	this->bind_socket(AF_INET, port, "127.0.0.1", sizeof(struct sockaddr_in));
	strcpy(this->psw, psw);
	strcpy(this->filename, filename);
}

void My_UDP_server::build_socket(int domain, int type, int protocol)
{
	this->sctfd = socket(domain, type, protocol);
	return;
}

int My_UDP_server::bind_socket(short family, u_short port, char* str_addr, int addrlen)
{
	if (sctfd == -1)
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
struct sockaddr_in My_UDP_server::get_sockaddr()
{
	return this->sock_addr;
}

void My_UDP_server::work_forever()
{
	int result;
	while(true)
	{
		result = start_routine();
	}
}


bool My_UDP_server::varify_password()
{
	cout << "receive password: " << msg_buffer.payload << " " << strlen(msg_buffer.payload) << endl;

	cout << "local password: " << this->psw << " " << strlen(this->psw) << endl;


	if (this->msg_buffer.pyld_length != strlen(this->msg_buffer.payload))
	{
		return false;
	}
	bool result;
	result = (strcmp(this->psw, this->msg_buffer.payload) == 0);

	return result;
}

int My_UDP_server::recv_join_req()
{
	cout << "start recv JOIN_REQ\n";
	int result;

	this->client_addr_length = sizeof(this->client_addr);
	result = recvfrom(this->sctfd, this->data_buffer, sizeof(data_buffer), 0, (struct sockaddr*) &(this->client_addr), &(this->client_addr_length));
	


	if (result != PACKSIZE)
	{
		return -1;
	}
	memset(&(this->msg_buffer), 0, sizeof(this->msg_buffer));
	result = unpack(data_buffer, &(this->msg_buffer));
	if (result != 0)
	{
		return result;
	}
	if (this->msg_buffer.id != JOIN_REQ)
	{
		return -1;
	}
	cout << "JOIN_REQ received\n";
	return 0;
}


int My_UDP_server::send_pass_req()
{
	cout << "start send PASS_REQ\n";
	int result;
	memset(&(this->msg_buffer), 0, sizeof(this->msg_buffer));
	this->msg_buffer.id = PASS_REQ;
	this->msg_buffer.pyld_length = 0;

	result = pack(data_buffer, &(this->msg_buffer));
	if (result != 0)
	{

		return result;
	}



	result = sendto(sctfd, this->data_buffer,PACKSIZE,0,(struct sockaddr*) &(this->client_addr),sizeof(this->client_addr));   
	if (result != PACKSIZE)
	{

		cout << result << endl;
		return -1;
	}
	cout << "PASS_REQ sent\n";
	return 0;
}

int My_UDP_server::recv_pass_resp()
{
	cout << "start recv PASS_RESP\n";
	int result;
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


	if (this->msg_buffer.id != PASS_RESP)
	{
		return -1;
	}
	if (this->msg_buffer.pyld_length > 50)
	{
		return -1;
	}

	cout << "PASS_RESP received\n";
	return 0;
}

int My_UDP_server::send_pass_accept()
{
	cout << "start send PASS_ACCEPT\n";
	int result;
	this->msg_buffer.id = PASS_ACCEPT;
	this->msg_buffer.pyld_length = 0;
	result = pack(this->data_buffer, &(this->msg_buffer));
	
	if (result != 0)
	{
		return result;
	}

	result = sendto(sctfd, this->data_buffer,PACKSIZE,0,(struct sockaddr*) &(this->client_addr),sizeof(this->client_addr));  
	if (result != PACKSIZE)
	{
		return -1;
	}
	cout << "PASS_ACCEPT sent\n";
	return 0;
}

int My_UDP_server::send_reject()
{
	cout << "start send REJECT\n";
	int result;
	this->msg_buffer.id = REJECT;
	this->msg_buffer.pyld_length = 0;
	result = pack(this->data_buffer, &(this->msg_buffer));
	
	if (result != 0)
	{
		return result;
	}

	result = sendto(sctfd, this->data_buffer,PACKSIZE,0,(struct sockaddr*) &(this->client_addr),sizeof(this->client_addr));  
	if (result != PACKSIZE)
	{
		return -1;
	}
	cout << "REJECT sent\n";
	return 0;

}

int My_UDP_server::send_data()
{
	// todo
	cout << "start send DATA\n";
	int result;
	uint32_t packet_count = 0;
	ifstream fin(this->filename, ios::binary);

	SHA1_Init(&(this->sha1_c));

	this->msg_buffer.id = DATA;

	while (true)
	{
		// somehow useless
		//memset(this->msg_buffer.payload, 0, sizeof(this->msg_buffer.payload));


		

		*((uint32_t*) this->msg_buffer.payload) = htonl(packet_count);



		fin.read((char*)(this->msg_buffer.payload + 4), sizeof(this->msg_buffer.payload) - 4);
		this->msg_buffer.pyld_length = fin.gcount();

		if (this->msg_buffer.pyld_length == 0)
		{
			break;
		}


		SHA1_Update(&(this->sha1_c), this->msg_buffer.payload + 4, this->msg_buffer.pyld_length);

		result = pack(this->data_buffer, &(this->msg_buffer));
	
		if (result != 0)
		{
			return result;
		}

		result = sendto(sctfd, this->data_buffer,PACKSIZE,0,(struct sockaddr*) &(this->client_addr),sizeof(this->client_addr));  
		if (result != PACKSIZE)
		{
			return -1;
		}
		cout << "DATA PACKET " << ntohl(*((uint32_t*) this->msg_buffer.payload)) << " sent\n";


		packet_count++;


	}
	cout << "DATA ALL sent\n";
	fin.close();
	return 0;
}


int My_UDP_server::send_terminate()
{
	cout << "start send TERMINATE\n";
	int result;

	memset(sha1_digest, 0, sizeof(sha1_digest));
	SHA1_Final(this->sha1_digest, &(this->sha1_c));


	this->msg_buffer.id = TERMINATE;
	this->msg_buffer.pyld_length = sizeof(this->sha1_digest);
	memcpy(this->msg_buffer.payload, this->sha1_digest, this->msg_buffer.pyld_length);

	




	result = pack(this->data_buffer, &(this->msg_buffer));
	
	if (result != 0)
	{
		return result;
	}

	result = sendto(sctfd, this->data_buffer,PACKSIZE,0,(struct sockaddr*) &(this->client_addr),sizeof(this->client_addr));  
	if (result != PACKSIZE)
	{
		return -1;
	}
	cout << "TERMINATE sent\n";
	return 0;
}

int My_UDP_server::start_routine()
{

	int result;

	
	result = this->recv_join_req();
	if (result != 0)
	{
		return result;
	}


	
	bool psw_pass = false;
	for (int req_count = 0; req_count < 3; req_count++)
	{
		result = this->send_pass_req();
		if (result != 0)
		{
			return result;
		}

		result = this->recv_pass_resp();
		if (result != 0)
		{
			return result;
		}

		if (varify_password())
		{
			cout << "password correct\n";
			psw_pass = true;
			break;
		}
		cout << "password incorrect\n";
	}

	if (!psw_pass)
	{
		cout << "password varification unpassed\n";
		result = this->send_reject();
		if (result != 0)
		{
			return -1;
		}
		return -2;
	}



	cout << "password varification passed\n";
	result = this->send_pass_accept();
	if (result != 0)
	{
		return result;
	}
	

	result = this->send_data();
	if (result != 0)
	{
		return result;
	}

	result = this->send_terminate();
	if (result != 0)
	{
		return result;
	}

	return 0;
}








int main(int argc, char** argv)
{


	My_UDP_server Test(atoi(argv[1]), argv[2], argv[3]);
	Test.work_forever();
	return 0;

}