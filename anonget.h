#ifndef ANONGET
#define ANONGET  0
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>  //for the damned memset(), stupid C crap.
#include <time.h>

#define HN_SIZE 63
#define PORT_LEN 6
#define SERV_QS 5
#define PORT_MIN 1024
#define PORT_MAX 65535
#define SSLIST_SIZE 2048
#define FBUFF_SIZE	512

int getRandomPort()
{
	return rand() % (PORT_MAX-PORT_MIN) + PORT_MIN;
}

int getPort(int c, char* v[])
{
	//check for port option.
	char *opt;
	int p = -1;
	if (c == 3)
	{
		if (getopt(c, v, "p:") == int('p'))
		{
			opt = optarg;
			p = atoi(optarg);
		}
		else		
       		p = -1;
		
		if (p < PORT_MIN || p > PORT_MAX)
		{
			printf("Port out of range. Choosing random port...\n");
			p = -1;
		}
	}
	else 
		p = -1;

	//If we don't have a valid port get a random one.
	if (p == -1)
	{
		//If they put too many or too few args let them know.
    	if ( (c > 1 && c < 3) || c > 3)
        {
			printf("Usage: ss [-p port]\n");
			printf("Choosing random port...\n");
		}
		//Just give them a random port.
		int r = getRandomPort();
		return r;
	}
	else
	{
		return p;	
	}
}


//Borrowed from Beej's guide used to "reap dead processes"
//http://beej.us/guide/bgnet/output/html/multipage/clientserver.html#simpleserver
void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

//Borrowed from Beej's guide used to get an IP address from a struct of addrinfo
//http://beej.us/guide/bgnet/output/html/multipage/clientserver.html#simpleserver
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

//This function is inpired by TWO StackOverFlow post:
//http://stackoverflow.com/questions/5594042/c-send-file-to-socket
//http://stackoverflow.com/questions/11952898/c-send-and-receive-file
//Usage:  Send a given file (fname), to a socket (outSock).
bool sendFileToSocket(char *fname, int outSock)
{
	
	FILE *fin = fopen(fname, "r");
	char fdata[FBUFF_SIZE];
    struct stat fstats;
	fstat(fin, &fstats); 
	
	if (fstats.st_size < 1)
	{
		printf("Error sending file '%s' 0 size?\n", fname);
		return false;
	}    

	//send the file length
	memset(fdata, '\0', FBUFF_SIZE);
	sprintf(fdata, "%d", fstats.st_size);

	//TODO loop>?
	int len = send(outSock, fdata, FBUFF_SIZE, 0);
	if (len < 0 )
	{
		printf("Could not send file size for '%s'\n", fname);
		return false;
	}
	
	size_t nbytes = 0;				//read this iteration
	size_t tbytes = 0;				//read total
	while ( (nbytes = fread(fdata, sizeof(char), FBUFF_SIZE, fin) ) > 0 /*&&
			(tbytes < fstats.st_size) */)
	{
		tbytes += nbytes;
		int offset = 0;
		int sent = 0;
		while ( (sent = send(outSock, fdata+offset, nbytes, 0) ) > 0 ||
				(sent == -1 && errno == EINTR) )
		{
			if (sent > 0) 
			{
				offset += sent;
				nbytes -= sent;
			}
		}
	}	
	
	fclose(fin);

	return true;
}

//this function is inspired by two StackOverFlow threads
//http://stackoverflow.com/questions/11952898/c-send-and-receive-file
//
//Usage: get a file (fname) from a socket (inSock).
bool recvFileFromSocket(char* fname, int inSock)
{
	FILE *fout = fopen(fname, "w");
	char fdata[FBUFF_SIZE];

	memset(fdata, '\0', FBUFF_SIZE);
	int len = recv(inSock, fdata, FBUFF_SIZE, 0);
	if (len < 0)
	{
		printf("error recieving file size for %s\n", fname);
	}
	int fsize = atoi(fdata);	
	
	int nbytes = 0;			//bytes this iteration
	int tbytes = 0; 		//total bytes
	do
	{
		nbytes = recv(inSock, fdata, FBUFF_SIZE, 0); 
		if (nbytes > 0)
		{
			fwrite(fdata, sizeof(char), nbytes, fout);
			tbytes += nbytes;	
		}
	} while (tbytes < fsize);

	fclose(fout);

	return true;
}
#endif
