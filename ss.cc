#include "anonget.h"

using namespace std;

int main(int argc, const char *argv[]) {
	char myName[HN_SIZE];
	char myIPstr[INET6_ADDRSTRLEN];	
	char incPort[PORT_LEN];    //the destination port for incoming connections.
	int conSock;		//connection sock. Socket for incoming connections on.
    srand(time(NULL));	
	
	int p = getPort(argc, argv);
	sprintf(incPort, "%d", p);	

	//get my name and set up a socket for incoming connections.
	gethostname(myName, HN_SIZE);
	getHostIP(myName, myIPstr);
	conSock = prepListenSocket(incPort); 
	
	//display Stepping Stone information
	printf("%s listening on	%s\n", myIPstr, incPort);
	
	//wait for a connection.
	if (listen(conSock, SERV_QS) == -1)
	{
		printf("Listen failed...");
		close(conSock);
		return 1;	
	}
	
	struct sigaction sa;
	sa.sa_handler = sigchld_handler;  //beej says reap dead processes	
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) 
	{
		printf("Error killing dead processes!?!?");
		close(conSock);
		exit(1);
	}	

	while (true) 
	{

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
			
		//translate the addr struct to a string with IP
		inet_ntop(incReqAddr.ss_family,
			getIP((struct sockaddr *)&incReqAddr),
			incReqIP, sizeof(incReqIP));
		
		if (!fork())
		{ 
			close(conSock);
			printf("Got a request from %s\n", incReqIP);
			//TODO close conSock for child
			//TODO recv() stepping stone list + request.
			char sslist[SSLIST_SIZE];
			memset(sslist, '\0', SSLIST_SIZE);	
			recv(incRequestSock, sslist, SSLIST_SIZE, 0);	
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
		
			strcat(sslist, " Hello! I'm Stepping Stone ");
			strcat(sslist, myIPstr); 
			if (send(incRequestSock, sslist, strlen(sslist), 0) == -1) 
			{
				printf("Error sending to %s", incReqIP);
			}		
			close(incRequestSock);
			exit(0);
		}
		close(incRequestSock);
	}

	close(conSock);
	return 0;
}
