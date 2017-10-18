//Knocker
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <cerrno>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define PACKET_SIZE 4096
using namespace std;
int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		cout << "Please give the required arguments. The usage is " << argv[0] << " configuration_file IP_address" << endl;
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
	}
	else
	{
		cout << "Could not open the file" <<endl;
		exit(1);
	}
	//checking the port numbers
	vector<int>::iterator it;
	// 	for(it = sequence.begin(); it != sequence.end(); it++)
	// 	{
	// 		cout <<*it<<" ";
	// 	}
	// cout<<endl;
	config_file.close();

	//Creating a socket
	int s = socket(PF_INET,SOCK_RAW,IPPROTO_TCP);
	if(s == -1)
	{
		cout << "Failed to create socket." << endl;
		exit(1);
	}
	char packet[PACKET_SIZE], *payload;
	memset(packet,0,PACKET_SIZE);
	struct iphdr *ipheader = (struct iphdr*) packet;
	struct tcphdr *tcpheader = (struct tcphdr*) (packet + sizeof(struct iphdr));
	payload = packet + sizeof(struct iphdr) + sizeof(struct tcphdr);
	strcpy(payload,"BLAH...BLAH...BLAH...BLAH");

	struct sockaddr_in source;
	source.sin_family = AF_INET;
	source.sin_port = htons(80);
	source.sin_addr.s_addr = inet_addr(argv[2]);

	//set the IP header values
	ipheader->ihl = 5;
	ipheader->version = 4;
	ipheader->tos = 0;
	ipheader->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr) + strlen(payload);
	ipheader->id = htonl(1234);
	ipheader->frag_off = 0;
	ipheader->ttl = 255;
	ipheader->protocol = IPPROTO_TCP;
	ipheader->check = 0;
	ipheader->saddr = inet_addr("1.2.3.4");
	ipheader->daddr = source.sin_addr.s_addr;

	//set the TCP header
	tcpheader->source = htons(1234);
	tcpheader->dest = htons(80);
	tcpheader->seq = 0;
	tcpheader->ack_seq= 0;
	tcpheader->doff = 5;
	tcpheader->fin = 0;
	tcpheader->syn = 1;
	tcpheader->rst = 0;
	tcpheader->ack = 0;
	tcpheader->urg = 0;
	tcpheader->window = htons(1234);
	tcpheader->check = 0;
	tcpheader->urg_ptr = 0;

	int one = 1;
	const int *val = &one;
	if(setsockopt(s,IPPROTO_IP,IP_HDRINCL,val,sizeof(one)) < 0)
	{
		perror("setsocketopt() error");
		close(s);
		exit(1);
	}

	size_t size = sequence.size();
	size_t x = 0;
	it = sequence.begin();
	while (x < size)
	{
		int port = *it;
		// cout << "the port number is " << port << endl;
		tcpheader->dest = htons(port);
		if(sendto(s,packet,ipheader ->tot_len,0,(struct sockaddr *)&source, sizeof(source)) < 0)
		{
			cout << "sendto failed";
		}
		else
		{
			x++;
			it++;
			// cout <<"sent" << x << endl ;
		}
	}
	close(s);
	return 0;
}
