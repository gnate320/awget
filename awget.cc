#include "anonget.h"


using namespace std;

int main(int argc, const char *argv[]) {

	srand(time(NULL));	
	char gangName[MAX_URL];
	if (argc < 2)
	{
		printf("Usage: awget URL\n");
		return 1;
	}
	else if (argc == 4)
	{
		//TODO get gang file option;

	}
	else
	{
		strcpy(gangName, "chaingang");
	}
	
	char myName[HN_SIZE];
	char myIPstr[INET6_ADDRSTRLEN];
	int toSS;
	int sent =0;

	//get my address information;
	gethostname(myName, HN_SIZE);
	getHostIP(myName, myIPstr);

	
	//TODO: pull random name from chaingang file;
	
	//BUFFER the chain gangfile
	char gang[SSLIST_SIZE];
	memset(gang, '\0', SSLIST_SIZE);
	struct stat fstats;
	stat(gangName, &fstats);
	int fsize = fstats.st_size;
	int nbytes =0;	
	int offset = 0;

	FILE *gIn = fopen(gangName, "r");
	while (nbytes = fread(gang+offset, sizeof(char), SSLIST_SIZE, gIn) > 0 &&
			(fsize > 0))
	{
		fsize -= nbytes;
		offset += nbytes;
	}
	
	//turn the gang data into a list
	char ** gangList = makeGangList(gang);
	
	//pick one
	int gangSize = atoi(gangList[0]);
	int r = getRandom(1, gangSize);
	
	char firstIP[INET6_ADDRSTRLEN];
	memset(firstIP, '\0', INET6_ADDRSTRLEN);
	char firstPORT[PORT_LEN];
	memset(firstPORT, '\0', PORT_LEN);
	char end[HN_SIZE];
	strcpy(end,"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.1234567890<>");

	offset = strspn(gangList[r], end)+1;
	strncpy(firstIP, gangList[r], offset-1);
	strncpy(firstPORT, gangList[r]+offset, PORT_LEN);

	toSS = prepConnectedSocket(firstIP, firstPORT);	
	
	//Send your gang to the first SS
	printf("sending chaingang...\n");
	sendFileToSocket(gangName, toSS);	

	//send size of (must send size to follow SS protocol)
	char request[MAX_URL];

	//send the request
	memset(request, '\0', MAX_URL);
	strcat(request, argv[1]);

		
	char confirm[MAX_URL];
	do
	{
		printf("sending request: %s\n", request);
		sendStringToSocket(request, strlen(request), toSS);
	
		//Get some sort of ACK
		memset(confirm, '\0', FBUFF_SIZE);
		recvStringFromSocket(confirm, toSS);
		
	}while ( strstr(confirm, "FAIL") );

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
