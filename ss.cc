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
	//pthread_mutex_lock(&lock);
	if (listen(conSock, SERV_QS) == -1)
	{
		printf("Listen failed...");
		close(conSock);
		return 1;	
	}
	//pthread_mutex_unlock(&lock);

	//TODO make runtime condition maybe....
	while (true)
	{
		ClientInfo clients[SERV_QS];
		pthread_t threads[SERV_QS];
		
		long tID = 0;
		int rc;
		while (tID < SERV_QS)	
		{
			struct sockaddr_storage incReqAddr;		//address info for incoming request
			socklen_t incReqSockSize = sizeof(incReqAddr);
			
			//pthread_mutex_lock(&lock);
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
			//pthread_mutex_unlock(&lock);
						
			//printf("Request: ");		
	
			clients[tID].cSock = incRequestSock;
			strcpy(clients[tID].myIP, myIPstr);
			strcpy(clients[tID].myName, myName);
			
			rc = pthread_create(&threads[tID], NULL,
					handleRequest, (void*) &clients[tID]);			
			if (rc)
			{
				printf("error creating thread\n");
			}

			tID++;
			//TODO: close sock ete or in thread?
			//close(incRequestSock);
		}

		//TODO collect threads
	}

	close(conSock);
	pthread_exit(NULL);
}
