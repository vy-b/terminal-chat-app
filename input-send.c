/* includes input thread and send thread 
input thread waits for keyboard input then adds to the list adt
signals send thread when input has been added and ready to send
*/
#include "input-send.h"
#include <pthread.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MSG_MAX_LEN 1024
#define PORT 22110

//defined in s-talk.c, passed here as pointers
static pthread_cond_t *s_pOkToSend;
static pthread_mutex_t *s_pmutex;
static List *s_pSendList;

pthread_t threadInput;
pthread_t threadSend;

// waits for input from keyboard and adds to list
void* inputThread(){
	while (1){
		char* msg[MSG_MAX_LEN];
		fgets(msg, MSG_MAX_LEN,stdin);

		// start critical section
		pthread_mutex_lock(s_pmutex);
		{
			//add to list
			if (List_add(s_pSendList, msg) != 0){
				perror("List add failed\n");
				exit(EXIT_FAILURE);
			}
		}
		pthread_mutex_unlock(s_pmutex);
		// end critical section
		
		// now wake up send thread
		pthread_mutex_lock(s_pmutex);
		{
			pthread_cond_signal(s_pOkToSend);
		}
		pthread_mutex_unlock(s_pmutex);
	}
}


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
		printf("message sent.\n");
	}
	return NULL;
}

void inputThread_init(pthread_mutex_t *pmutex, pthread_cond_t *pOkToSend, List* pSendList)
{
    // store the parameters in the pointers that were initialized at the beginning
    s_pmutex = pmutex;
    s_pOkToSend = pOkToSend;
    s_pSendList = pSendList;

    pthread_create(&threadInput, NULL, inputThread, NULL);
}

void inputThread_shutdown()
{
    pthread_join(threadInput,NULL);
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