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
	
	
	close(conSock);
	return 0;
}
