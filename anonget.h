#ifndef ANONGET
#define ANONGET  0
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  //for the damned memset(), stupid C crap.
#include <time.h>

#define HN_SIZE 63
#define PORT_LEN 6
#define SERV_QS 5
#define PORT_MIN 1024
#define PORT_MAX 65535
#define SSLIST_SIZE 1000

int getRandomPort()
{
	return rand() % (PORT_MAX-PORT_MIN) + PORT_MIN;
}

void *getIP(struct sockaddr * sa)
{
	if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int prepSocket(const char* hostname, const char* port,
 	char *ipstr, int strsize, bool bFlag)
{
	struct addrinfo hints;
    struct addrinfo *myAddrResults, *rp;	
	void *addr;
	int sockFD = -1;
	int yes = 1;

	memset(&hints, 0, sizeof(struct addrinfo));	
	hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

	
	if (getaddrinfo(hostname, port, &hints, &myAddrResults) != 0)
	{
		printf("Error establshing socket to host named %s on port %s", 
			hostname, port);
		return -1;
	} 
	
	for (rp = myAddrResults; rp != NULL; rp = rp->ai_next)
	{
		sockFD = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sockFD == -1)
		{
			printf("Could not bind host to a socket.");
			continue;
		}
			
		if (bFlag)
		{
			if (setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR,
				&yes, sizeof(int)) == -1)
			{
				printf("Could not set the socket options.");
				close(sockFD);
				return -1;
			}
			if (bind(sockFD, rp->ai_addr, rp->ai_addrlen) == -1)
			{	
				close(sockFD);
				printf("Could not bind host %s to port %s.", hostname, port);
				continue;
			}
		}
		else
		{
			if (connect(sockFD, rp->ai_addr, rp->ai_addrlen) == -1) 
			{
            	close(sockFD);
            	printf("Could not connect to %s to port %s.", hostname, port);
            	continue;
        	}
		}
		
		//get the addr as a string.
		if (rp->ai_family == AF_INET)  //IPv4 addr
		{
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)rp->ai_addr;
			addr = &(ipv4 ->sin_addr);
		}
		else						   //IPV6 addr
		{
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)rp->ai_addr;
			addr = &(ipv6->sin6_addr);
		}
				
		break;
	}
	
	if (rp == NULL) 
	{
		printf("could not locate host %s", hostname);
		return -1;
	}

	//conver IP to string
	inet_ntop(rp->ai_family, addr, ipstr, strsize);

	freeaddrinfo(myAddrResults);
	
	return sockFD;
}

#endif
