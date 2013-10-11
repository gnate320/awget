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

int getHostIP(const char* hostname, char *ip)
{
	struct addrinfo hints, *res, *rp;
    int status;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((status = getaddrinfo(hostname, NULL, &hints, &res) !=0 ))
	{
		printf("Could not find an IP address for %s", hostname);
		return -1;
	}	

	for(rp = res; rp != NULL; rp = rp->ai_next)
	{
		void *addr;
		if (rp->ai_family == AF_INET)
		{
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)rp->ai_addr;
			addr = &(ipv4->sin_addr);
		}
		else
		{
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)rp->ai_addr;
			addr = &(ipv6->sin6_addr);
		}

		if (inet_ntop(rp->ai_family, addr, ip, INET6_ADDRSTRLEN) != NULL)
			return 0;
	}	

	return -1;
}

int prepListenSocket(const char* port)
{
	struct addrinfo hints;
    struct addrinfo *servResults, *rp;	
	int sockFD = -1;
	int yes = 1;

	memset(&hints, 0, sizeof(struct addrinfo));	
	hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, port, &hints, &servResults) != 0)
	{
		printf("Error establshing socket for accepting connections on port %s", port);
		return -1;
	} 
	
	for (rp = servResults; rp != NULL; rp = rp->ai_next)
	{
		sockFD = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sockFD == -1)
		{
			printf("Could not bind host to a socket.");
			continue;
		}
			
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
			printf("Could not bind to port %s.", port);
			continue;
		}
				
		break;
	}
	
	if (rp == NULL) 
	{
		printf("Unable to create listening socket on port %s", port);
		return -1;
	}

	freeaddrinfo(servResults);
	
	return sockFD;
}


int prepConnectedSocket(const char* hostname, const char* port)
{
	struct addrinfo hints;
    struct addrinfo *myAddrResults, *rp;	
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
				
		if (connect(sockFD, rp->ai_addr, rp->ai_addrlen) == -1) 
		{
            //close(sockFD);
            printf("Could not connect to %s to port %s.", hostname, port);
            continue;
        }	
				
		break;
	}
	
	if (rp == NULL) 
	{
		printf("could not locate host %s", hostname);
		return -1;
	}

	freeaddrinfo(myAddrResults);
	
	return sockFD;
}

#endif
