#include <stdio.h>
#include <stdlib.h>

#include "sendThread.h"

#include "list.h"
#include <pthread.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

//defined in s-talk.c, passed here as pointers
static pthread_cond_t *s_pOkToSend;
static pthread_mutex_t *s_pmutex;
static *s_pSendList;

pthread_t threadSend;

// takes item off sendlist and send
void* sendThread(){
	// can initialize socket first (and THEN can wait for signal to send)
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
		//after creating sockets, wait for signal to send (wait for item to be added to list)
		pthread_mutex_lock(s_pmutex);
		{
			pthread_cond_wait(s_pOkToSend, s_pmutex);
		}
		pthread_mutex_unlock(s_pmutex);
		
		char msg[MSG_MAX_LEN];

		pthread_mutex_lock(s_pmutex);
		{
			// take item off list and store in string
			msg = List_remove(s_pSendList);
		}
		pthread_mutex_unlock(s_pmutex);

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

void sendThread_init(pthread_mutex_t *pmutex, pthread_cond_t *pOkToSend, List* pSendList)
{
    // store the parameters in the pointers that were init at the beginning
    // of this file
    s_pmutex = pmutex;
    s_pOkToSend = pOkToSend;
    s_pSendList = pSendList;

    pthread_create(&threadSend, NULL, sendThread, NULL);
}

void sendThread_shutdown()
{
    pthread_join(threadSend,NULL);
}