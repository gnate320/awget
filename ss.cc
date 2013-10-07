#include "anonget.h"

using namespace std;

int main(int argc, const char *argv[]) {
	char hostname[HN_SIZE];
	char myIPstr[INET6_ADDRSTRLEN];	

	gethostname(hostname, HN_SIZE);

	char port[PORT_LEN];
	port [0] = '3';
	port[1] = '4';
    port[2] = '5';
    port[3] = '4';
    port[4] = '\0'; 	

	prepSocket(hostname, port, myIPstr, sizeof(myIPstr)); 
	
	printf("%s listening on	%s", myIPstr, port);

	return 0;
}
