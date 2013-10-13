#include "anonget.h"


using namespace std;

int main(int argc, const char *argv[]) {
	if (argc < 2)
	{
		printf("Usage: awget URL\n");
		return 1;
	}
	
	char myName[HN_SIZE];
	char myIPstr[INET6_ADDRSTRLEN];
	int toSS;
	int sent =0;

	//get my address information;
	gethostname(myName, HN_SIZE);
	getHostIP(myName, myIPstr);

	
	//TODO: pull random name from chaingang file;
	//toSS = prepConnectedSocket("cabbage.cs.colostate.edu", "4649");	
	toSS = prepConnectedSocket("Nataku-U52F", "4649");	
	
	//Send your gang to the first SS
	printf("sending chaingang...\n");
	sendFileToSocket("chaingang", toSS);	

	//send size of (must send size to follow SS protocol)
	char request[FBUFF_SIZE];
	//memset(request, '\0', FBUFF_SIZE);
	//sprintf(request, "%u", strlen(argv[1]));
	//printf("sending: %s\n", request);
	//send(toSS, request, FBUFF_SIZE, 0);	

	//send the request
	memset(request, '\0', FBUFF_SIZE);
	strcat(request, argv[1]);
	printf("sending request: %s\n", request);
	send(toSS, request, FBUFF_SIZE, 0);

	printf("waiting for response..\n");	

	//char reply[SSLIST_SIZE];
	int numbytes;
	if ((numbytes = recv(toSS, request, SSLIST_SIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }
	
	printf("Stepping Stone says: %s\n", request);

	close(toSS);
}
