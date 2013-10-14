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
#include <pthread.h>

#define HN_SIZE 63
#define PORT_LEN 6
#define SERV_QS 5
#define PORT_MIN 1024
#define PORT_MAX 65535
#define SSLIST_SIZE 6144
#define MAX_URL 1024
#define FBUFF_SIZE	512
#define MAX_FILE 1073741824

//GLobal locks
pthread_mutex_t lock;

typedef struct
{
	int	cSock;
	char myIP[INET6_ADDRSTRLEN];
	char myName[HN_SIZE];

} ClientInfo;

int getRandom(int h, int l)
{
	return rand() % (h-l) + l;
}

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
bool sendStringToSocket(char *message, int size, int outSock)
{
	
	char fdata[FBUFF_SIZE];
    //struct stat fstats;
	//stat(fname, &fstats); 
	
	if (size < 1)
	{
		printf("Error sending string '%s' 0 size?\n", message);
		return false;
	}    

	//send the file length
	memset(fdata, '\0', FBUFF_SIZE);
	//printf("sending size: %u\n", size);
	sprintf(fdata, "%u", size);
	//printf("sent as: %s\n", fdata);

	//TODO loop>?
	int len = 0;
	do
	{
		len = send(outSock, fdata, FBUFF_SIZE, 0);
	}while (len == -1);

	if (len < 0 )
	{
		printf("Could not send string size for '%s'\n", message);
		return false;
	}

	//printf("sending...\n");	
	size_t nbytes = 0;				//read this iteration
	size_t tbytes = 0;				//read total
	while ( tbytes < size )
	{
		strncpy(fdata, message+tbytes, FBUFF_SIZE);
		tbytes += FBUFF_SIZE;
		nbytes = FBUFF_SIZE;
		//printf("read: %d, totalread: %d\n", FBUFF_SIZE, tbytes);
		int offset = 0;
		int sent = 0;		//sent this interation
		while ( ((sent = send(outSock, fdata+offset, FBUFF_SIZE-offset, 0) ) > 0 				|| (sent == -1 && errno == EINTR)) && (nbytes > 0) )
		{
			if (sent > 0) 
			{
				offset += sent;
				nbytes -= sent;
			}
			
			//printf("sent: %d, w/offSet: %d, remain: %d\n", sent, offset-sent, nbytes);
		}
	}		

	return true;
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
	stat(fname, &fstats); 
	
	if (fstats.st_size < 1)
	{
		printf("Error sending file '%s' 0 size?\n", fname);
		return false;
	}    

	//send the file length
	memset(fdata, '\0', FBUFF_SIZE);
	//printf("sending size: %u\n", fstats.st_size);
	sprintf(fdata, "%u", fstats.st_size);
	//printf("sent as: %s\n", fdata);

	//TODO loop>?
	int len =0;
	do
	{
		len = send(outSock, fdata, FBUFF_SIZE, 0);
	}while(len == -1);

	if (len < 0 )
	{
		printf("Could not send file size for '%s'\n", fname);
		return false;
	}

	//printf("sending...\n");	
	size_t nbytes = 0;				//read this iteration
	size_t tbytes = 0;				//read total
	while ( (nbytes = fread(fdata, sizeof(char), FBUFF_SIZE, fin) ) > 0  &&
			(tbytes < fstats.st_size) )
	{
		tbytes += nbytes;
		//printf("read: %d, totalread: %d\n", nbytes, tbytes);
		int offset = 0;
		int sent = 0;		//sent this interation
		while ( ((sent = send(outSock, fdata+offset, nbytes, 0) ) > 0 ||
				(sent == -1 && errno == EINTR)) && (nbytes > 0) )
		{
			if (sent > 0) 
			{
				offset += sent;
				nbytes -= sent;
			}
			
			//printf("sent: %d, w/offSet: %d, remain: %d\n", sent, offset-sent, nbytes);
		}
	}	
	
	fclose(fin);

	return true;
}

//this function is inspired by two StackOverFlow threads
//http://stackoverflow.com/questions/11952898/c-send-and-receive-file
//http://stackoverflow.com/questions/5594042/c-send-file-to-socket
//Usage: get a file (fname) from a socket (inSock).
bool recvFileFromSocket(char* fname, int inSock)
{
	FILE *fout = fopen(fname, "w");
	char fdata[FBUFF_SIZE];
	memset(fdata, '\0', FBUFF_SIZE);
	
	int nbytes = 0;
	int rbytes = FBUFF_SIZE;

	//printf("About to recv on socket: %d\n", inSock);
	

	int fsize = 0;		
	pthread_mutex_lock(&lock);
	do	
	{	
		nbytes = recv(inSock, fdata, FBUFF_SIZE, 0); 
		if (nbytes > 0)
		{	
			rbytes -= nbytes;
		}
	}while ( rbytes > 0 );
	pthread_mutex_unlock(&lock);

	fsize = atoi(fdata);
	
	nbytes = 0;			//bytes this iteration
	rbytes = fsize; 	//remaining bytes

	//printf("file size is %d\n",fsize);
	if (fsize == 0)
		return false;	
	
	do 
	{
		nbytes = recv(inSock, fdata, FBUFF_SIZE, 0);
		if (nbytes > 0)
		{
			if (rbytes < FBUFF_SIZE)//recv 512 bytes but that maybe too much
				nbytes = rbytes;	
			fwrite(fdata, sizeof(char), nbytes, fout);
			rbytes -= nbytes;	
			//printf("wrote bytes %d to file?>?\n", nbytes);
		}
		//printf("recv: %d, remain: %d\n", nbytes, rbytes);
	}while ( rbytes > 0 );


	fclose(fout);

	return true;
}

bool recvStringFromSocket(char* message, int inSock)
{
	char fdata[FBUFF_SIZE];
	memset(fdata, '\0', FBUFF_SIZE);
	
	int nbytes = 0;
	int rbytes = FBUFF_SIZE;
		
	int fsize = 0;
	pthread_mutex_lock(&lock);
	do	
	{	
		nbytes = recv(inSock, fdata, FBUFF_SIZE, 0); 
		if (nbytes > 0)
		{	
			rbytes -= nbytes;
		}
	}while ( rbytes > 0 );
	pthread_mutex_unlock(&lock);
	fsize = atoi(fdata);	
   	
	//printf("data came in as %s\n", fdata); 	
	//printf("file size recved is %d\n",fsize);
	
	if (fsize == 0)
		return false;	

	nbytes = 0;			//bytes this iteration
	rbytes = fsize; 		//remaining bytes
	
	do 
	{
		nbytes = recv(inSock, fdata, FBUFF_SIZE, 0);
		if (nbytes > 0)
		{
			if (rbytes < FBUFF_SIZE)//recv 512 bytes but that maybe too much
				nbytes = rbytes;	
			strncat(message, fdata, nbytes);
			rbytes -= nbytes;	
		}
		//printf("recv: %d, remain: %d\n", nbytes, rbytes);
	}while ( rbytes > 0 );

	
	//printf("received: %s\n", message);		
	return true;
}

char** makeGangList(char *gdata)
{

	char* s = gdata;
	char end[67];
	//valid characters in an addess and port (list entry)
	strcpy(end,"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.,1234567890<>");

	//printf("inside make gang list buffer is:\n%s\n", s);
	
	int count = 0;
	int span = 0;
	int i = 0;

	//grab the number of nodes form the list
	char temp[PORT_LEN];
	memset(temp, '\0', PORT_LEN);
	span = strspn(s, end);
	strncpy(temp, s, span);
	count = atoi(temp);
	//printf("span is:%d\n", span);	
	//printf("size is:%d\n", count);
	//printf("s points to %s\n", s);
	
		
	//create array of correct size
	char** gang = (char**)malloc(sizeof(char*) * count+1);
	if (gang == NULL)
		return NULL;
	
	//create the first element stuff in it the size of the list
	gang[0] = (char*) malloc(PORT_LEN);
	if (gang[0] == NULL)
		return NULL;
	memset(gang[0], '\0', PORT_LEN);
	strcpy(gang[0], temp);

	//move up the list
	s = s+(span+1)*sizeof(char);
	i++;
	
	//printf("gang[0]: %s\n", gang[0]);
	while (i < count+1) 
	{
		gang[i] = (char*) malloc(INET6_ADDRSTRLEN+PORT_LEN+3);
		if (gang[i] == NULL)
			return NULL;
		memset(gang[i], '\0', (INET6_ADDRSTRLEN+PORT_LEN+3));
		span = strspn(s, end);
		strncpy(gang[i], s, span);
		//printf("span is: %d\n", span);
		//printf("gang[%d]: %s\n", i, gang[i]);
		i++;
		if(i < count+1)
			s = s+(span+1)*sizeof(char);
	}
	
			
	return gang;
}

char** cleanGangList(char** gang)
{
	int size = atoi(gang[0]);
	
	for (int i = 1; i < size+1; ++i)
	{
		free(gang[i]);
	}

	free(gang[0]);
	free(gang);
	return NULL;
}

void *handleRequest(void *c)
{
	ClientInfo myC;
	memcpy(&myC, (ClientInfo*)c, sizeof(ClientInfo));

	printf("Request: ");
	
	//recv() stepping stone list + request.
	
	char sslist[SSLIST_SIZE];
	memset(sslist, '\0', SSLIST_SIZE);	
	recvStringFromSocket(sslist, myC.cSock);		
	
	char request[MAX_URL];
	bool gotit	= false;
	do
	{
		//printf("waiting for request\n");
		memset(request, '\0', MAX_URL);
		gotit = recvStringFromSocket(request, myC.cSock);	

		//send some sort of ACK
		char confirm[FBUFF_SIZE];
		memset(confirm, '\0', FBUFF_SIZE);
		if (gotit)
			sprintf(confirm, "PASS");
		else
			sprintf(confirm, "FAIL");
		
		sendStringToSocket(confirm, strlen(confirm), myC.cSock);
	}while (!gotit);

			

	printf("%s...\n", request);	
	printf("Chainlis is: \n%s\n", sslist);
			
	//convert SSlist data to array of SS data
	char** ourGang = NULL;
	ourGang = makeGangList(sslist);
	
	
	// remove THIS Stepping stone IP form the list
	int gangSize = atoi(ourGang[0]);
	int myIndex;
	char* nextIP;
	char* nextPort;

	for (int i = 1; i < gangSize+1; ++i)
	{
		//printf("gang[%d] = %s\n", i, ourGang[i]);
		if ( strstr(ourGang[i], myC.myIP) )
		{
			myIndex = i;
			break;
		}  
		else if ( strstr(ourGang[i], myC.myName) )
		{
			myIndex = i;
			break;
		}
	}	

	//Put this SS at the end of the list;
	nextIP = ourGang[myIndex];
	ourGang[myIndex] = ourGang[gangSize];
	ourGang[gangSize] = nextIP;
	gangSize--;
	
	char relay[SSLIST_SIZE];
	memset(relay, '\0', SSLIST_SIZE);
		
	if (gangSize > 0)
	{
		//TODO get random SS
		
		char end[67];
		memset(end, '\0', 67);
		strcpy(end,"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.1234567890<>");
		nextIP = (char*) malloc(INET6_ADDRSTRLEN);
		memset(nextIP, '\0', INET6_ADDRSTRLEN);
		nextPort = (char*) malloc(PORT_LEN);
		memset(nextPort, '\0', PORT_LEN);
		int offset = strspn(ourGang[1], end)+1;
		strncpy(nextIP, ourGang[1], offset-1);
		strncpy(nextPort, ourGang[1]+offset, PORT_LEN);
		
		printf("next SS is <%s,%s>\n", nextIP, nextPort);
		
		//TODO Connect to next SS
		int nextSS = prepConnectedSocket(nextIP, nextPort);

		//TODO send SS list with request

		char passableSSList[SSLIST_SIZE];
		memset(passableSSList, '\0', SSLIST_SIZE);
		sprintf(passableSSList, "%d\n", gangSize);
		//strcat(passableSSList, gangSize+"\n");
		for (int i = 1; i < gangSize+1; i++)
		{
			strcat(passableSSList, ourGang[i]);
		}
		
		//sendList
		sendStringToSocket(passableSSList, strlen(passableSSList), nextSS);

		//send request
		
		char confirm[FBUFF_SIZE];
		do
		{
			sendStringToSocket(request, strlen(request), nextSS);
			
			//Get some sort of ACK
			memset(confirm, '\0', FBUFF_SIZE);
			recvStringFromSocket(confirm, nextSS);
		
	    }while ( strstr(confirm, "FAIL") );

		
		//free(passableSSList);
		free(nextIP);
		free(nextPort);		

		printf("waiting for file...\n...\n");		

		//get file name
		//recvStringFromSocket(relay, nextSS);
		bool gotit	= false;
		do
		{
			//printf("waiting for request\n");
			memset(relay, '\0', MAX_URL);
			gotit = recvStringFromSocket(relay, nextSS);	

			//send some sort of ACK
			char confirm[FBUFF_SIZE];
			memset(confirm, '\0', FBUFF_SIZE);
			if (gotit)
				sprintf(confirm, "PASS");
			else
				sprintf(confirm, "FAIL");
		
			sendStringToSocket(confirm, strlen(confirm), nextSS);
		}while (!gotit);

		//get the file
		recvFileFromSocket(relay, nextSS);

		//send file name
		do
		{
			sendStringToSocket(relay, strlen(relay), myC.cSock);
			
			//Get some sort of ACK
			memset(confirm, '\0', FBUFF_SIZE);
			recvStringFromSocket(confirm, myC.cSock);
		
	    }while ( strstr(confirm, "FAIL") );
			

		//send the file to the client
		printf("Relaying the file...\n");
		sendFileToSocket(relay, myC.cSock);			

	}
	//else lastSS
	else
	{
		printf("chainlist is empty\n");
		printf("issueing wget for <%s>\n", request);

		//DO THE CALL!!system.wget
		char url [MAX_URL];
		memset(url, '\0', MAX_URL);
		strcat(url, "wget ");
		strcat(url, request);
		system(url);
			
		printf("File received!\n"); 
			
		printf("Relaying file...\n");	
		
		//TODO find the result!!
		char *fname = strrchr(request, '\');
		if (fname == NULL)
			fname = "index.html;
		
		//send name;
		char confirm[FBUFF_SIZE];
		do
		{
			sendStringToSocket(fname, strlen(fname), myC.cSock);
			
			//Get some sort of ACK
			memset(confirm, '\0', FBUFF_SIZE);
			recvStringFromSocket(confirm, myC.cSock);
		
	    }while ( strstr(confirm, "FAIL") );

		//send file;
		sendFileToSocket(fname, myC.cSock);		
	}

		
	printf("Goodbye!\n");

	//TODO:  best place to free gang>?  properly freed gang?
	ourGang = cleanGangList(ourGang);		
	close(myC.cSock);
	pthread_exit(NULL);
}

#endif
