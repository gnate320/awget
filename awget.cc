#include "anonget.h"

using namespace std;

int main(int argc, const char *argv[]) {
	char myName[HN_SIZE];
	char myIPstr[INET6_ADDRSTRLEN];
	int toSS;

	//get my address information;
	gethostname(myName, HN_SIZE);
	getHostIP(myName, myIPstr);

	
	//TODO: pull random name from chaingang file;
	//toSS = prepConnectedSocket("cabbage.cs.colostate.edu", "4649");	
	toSS = prepConnectedSocket("Nataku-U52F", "4649");	
	
	//Send your gang to the first SS
	sendFileToSocket("chaingang", toSS);	

	//TODO:Check actual request from CL
	char request[MAX_URL];
	memset(request, '\0', MAX_URL);
	strcat(request, argv[1]);
	send(toSS, request, MAX_URL, 0);
	
	char reply[SSLIST_SIZE];
	int numbytes;
	if ((numbytes = recv(toSS, reply, SSLIST_SIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }
	
	printf("Stepping Stone says: %s\n", reply);

	close(toSS);
}
