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
List* pList = List_create();

void* inputThread(){
	char* msg[MSG_MAX_LEN];
	fgets(msg, MSG_MAX_LEN,stdin);

	// need to implement: condition variables to check if list is full before adding

	// start critical section
	pthread_mutex_lock(&mutex);
	//add to list
	List_add(pList, msg);
	pthread_mutex_unlock(&mutex);
	// end critical section
}

void* sendThread(){
	char* received[MSG_MAX_LEN];

	pthread_mutex_lock(&mutex);
	// take item off list and store in string
	received = List_remove();
	pthread_mutex_unlock(&mutex);

	int socketDescriptor;
	// socket address for sender (self) and receiver (remote address)
	struct sockaddr_in sin, sinRemote;

	memset(&sin,0,sizeof(sin));
	memset(&sinRemote, 0, sizeof(sinRemote));

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
	 
	// currently no while loop
	sendto(socketDescriptor, received, MSG_MAX_LEN,0, (struct sockaddr*) &sinRemote, sizeof(sinRemote));
	printf("Message sent.\n"); // for debugging purposes
	
}