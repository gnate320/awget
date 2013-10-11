#include "anonget.h"

using namespace std;

int main(int argc, const char *argv[]) {
	char myName[HN_SIZE];
	char myIPstr[INET6_ADDRSTRLEN];
	int toSS;

	gethostname(myName, HN_SIZE);
	getHostIP(myName, myIPstr);
	toSS = prepConnectedSocket("cabbage.cs.colostate.edu", "4649");	

	printf("connected?");
	
	send(toSS, myIPstr, strlen(myIPstr), 0);

	char reply[SSLIST_SIZE];
	int numbytes;
	if ((numbytes = recv(toSS, reply, SSLIST_SIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }
	
	printf("Stepping Stone says: %s\n", reply);

	close(toSS);
}
