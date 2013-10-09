#include "anonget.h"

using namespace std;

int main(int argc, const char *argv[]) {
	char myIP[INET6_ADDRSTRLEN];
	int toSS;

	toSS = prepSocket("cabbage.cs.colostate.edu", "4649", myIP, sizeof(myIP), false);

	printf("connected?");
	
	send(toSS, myIP, strlen(myIP), 0);

	char reply[SSLIST_SIZE];
	int numbytes;
	if ((numbytes = recv(toSS, reply, SSLIST_SIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }
	
	printf("Stepping Stone says: %s\n", reply);

	close(toSS);
}
