#include "anonget.h"

using namespace std;

int main(int argc, const char *argv[]) {
	char myName[HN_SIZE];
	char myIPstr[INET6_ADDRSTRLEN];	
	char incPort[PORT_LEN];    //the destination port for incoming connections.
	int conSock;		//connection sock. Socket for incoming connections on.
    srand(time(NULL));
	
	//check for port option.
	int p = -1;
	char* opt;
	if (argc == 3)
	{
		if (getopt(argc, argv, "p:") == int('p'))
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
    	if ( (argc > 1 && argc < 3) || argc > 3)
        	printf("Unrecognized command line arguments. Choosing random port...");
		//Just give them a random port.
		int r = getRandomPort();
		sprintf(incPort, "%d", r);  
	}
	else
	{	
		sprintf(incPort, "%d", p);
	}	
	
	//get my name and set up a socket for incoming connections.
	gethostname(myName, HN_SIZE);
	conSock = prepSocket(myName, incPort, myIPstr, sizeof(myIPstr)); 
	
	//display Stepping Stone information
	printf("%s listening on	%s\n", myIPstr, incPort);
	
	//wait for a connection.
	if (listen(conSock, SERV_QS) == -1)
	{
		printf("Listen failed...");
		close(conSock);
		return 1;	
	}
	
	while (1) 
	{
		printf("Stepping Stone waiting for requests...");

		struct sockaddr_storage incReqAddr;		//address info for incoming request
		socklen_t incReqSockSize = sizeof(incReqAddr);
		int incRequestSock = accept(conSock, (struct sockaddr *) &incReqAddr, &incReqSockSize);
		char incReqIP[INET6_ADDRSTRLEN];

		if (incRequestSock == -1)
		{
			printf("Error accepting request...");
			close(conSock);
			close(incRequestSock);
			//TODO:  Don't exit program, just jump back to top of loop
			return 1;
		}
			
		//translate the addr struct to an string with IP
		inet_ntop(incReqAddr.ss_family,
			getIP((struct sockaddr *)&incReqAddr),
			incReqIP, sizeof(incReqIP));
		printf("Got a request from %s", incReqIP);
		//TODO FORK HERE! 
		//TODO close conSock for child
		//TODO recv() stepping stone list + request.
		//TODO remove THIS Stepping stone IP form the list
			
		//TODO if !lastSS :
			//TODO look up next SS
			//TODO Connect to next SS
			//TODO send SS list with request
			//TODO recv() file "package" as "result"
		//TODO else lastSS
			//TODO system.wget(request)
			//TODO "package" as "result"
			
		//TODO send(result) to incRequestSock
		
		char sslist[SSLIST_SIZE];
		memset(sslist, '\0', SSLIST_SIZE);
		strcat(sslist, "hello, from ");
		strcat(sslist, myIPstr); 
		if (send(incRequestSock, sslist, strlen(sslist), 0) == -1) 
		{
			printf("Error sending to %s", incReqIP);
		}		
		close(incRequestSock);
	}

	close(conSock);
	return 0;
}
