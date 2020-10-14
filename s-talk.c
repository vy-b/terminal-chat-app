
#include <stdio.h>
#include <stdlib.h>

#include "sendThread.h"

#include "list.h"
#include <pthread.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

// initialize everything here, pass in other files
static pthread_cond_t s_OkToSend = PTHREAD_COND_INITIALIZER;
static pthread_cond_t s_OkToPrint = PTHREAD_COND_INITIALIZER;

pthread_mutex_t s_mutex = PTHREAD_MUTEX_INITIALIZER;

List* SendList = List_create();
List* ReceiveList = List_create();

/* need to encapsulate these
void* receiveThread(){

	int socketDescriptor;
	// socket address for receiver (self)
	struct sockaddr_in sin;

	memset(&sin,0,sizeof(sin));

	// filling sender information
	sin.sin_family = AF_INET; //IPv4 - don't need to implement IPv6
	sin.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(PORT);

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
	 
	while(1){
		// socket address of sender (remote)
		struct sockaddr_in sinRemote;
		memset(&sinRemote, 0, sizeof(sinRemote));
		unsigned int sin_len = sizeof(sinRemote);

		char received[MSG_MAX_LEN];

		if ( recvfrom(socketDescriptor, received, MSG_MAX_LEN, 0, (struct sockaddr*) &sinRemote, &sin_len < 0 ) {
			perror("writing to socket failed\n");
			exit(EXIT_FAILURE);
		}
		// entering critical section
		pthread_mutex_lock(&mutex);
		// add received item to list to print
		List_add(pReceiveList, received);
		pthread_mutex_unlock(&mutex);
		// done critical section
	}
	return NULL;
}

void* printThread() {
	char toPrint[MSG_MAX_LEN];

	// entering critical section
	pthread_mutex_lock(&mutex);
	// add received item to list to print
	toPrint = List_remove(pReceiveList);
	pthread_mutex_unlock(&mutex);
	// done critical section

	printf("%s\n",toPrint);
	return NULL;
}
*/
int main() {
	sendThread_init(&s_mutex, &s_OkToSend, SendList);
	inputThread_init(&s_mutex, &s_OkToSend, SendList);

	sendThread_shutdown();
	inputThread_shutdown();
}