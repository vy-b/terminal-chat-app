#include <stdio.h>
#include <stdlib.h>

#include "list.h"
#include <pthread.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define MSG_MAX_LEN 1024
#define PORT 22110

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
List* pSendList = List_create();
List* pReceiveList = List_create();

// waits for input from keyboard and adds to list
void* inputThread(){
	char* msg[MSG_MAX_LEN];
	fgets(msg, MSG_MAX_LEN,stdin);

	// need to implement: condition variables to check if list is full before adding

	// start critical section
	pthread_mutex_lock(&mutex);
	//add to list
	List_add(pSendList, msg);
	pthread_mutex_unlock(&mutex);
	// end critical section
}

// takes item off sendlist and send
void* sendThread(){
	char msg[MSG_MAX_LEN];

	pthread_mutex_lock(&mutex);
	// take item off list and store in string
	msg = List_remove(pSendList);
	pthread_mutex_unlock(&mutex);

	int socketDescriptor;
	// socket address for sender (self)
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
		// socket address of receiver (remote address)
		struct sockaddr_in sinRemote;
		memset(&sinRemote, 0, sizeof(sinRemote));
		if ( sendto(socketDescriptor, msg, MSG_MAX_LEN,0, (struct sockaddr*) &sinRemote, sizeof(sinRemote)) < 0 ) {
			perror("writing to socket failed\n");
			exit(EXIT_FAILURE);
		}
		printf("message sent.\n")
	}
	return NULL;
}

// 
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