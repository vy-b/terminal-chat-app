#include <pthread.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 22110

int socket_init(){
    int socketDescriptor;
	// socket address for receiver (self)
	struct sockaddr_in sin;

	memset(&sin,0,sizeof(sin));

	// filling sender information
	sin.sin_family = AF_INET; //IPv4 - don't need to implement IPv6
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(PORT);

	// initialize socket + error check
	if ( (socketDescriptor = socket(PF_INET, SOCK_DGRAM,0)) < 0){
		perror("socket creation failed\n");
		exit(EXIT_FAILURE);
	}

    // bind
    if ( bind(socketDescriptor, (struct sockaddr*) &sin, sizeof(sin)) < 0){
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }
    return socketDescriptor;
}
	 