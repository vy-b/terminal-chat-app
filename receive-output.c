#include "receive-output.h"

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
static pthread_cond_t *s_pOkToPrint;
static pthread_mutex_t *s_pmutex;
static List *s_pPrintList;

pthread_t threadPrint;
pthread_t threadReceive;

void* receiveThread(){

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
	 
	while(1){
		// socket address of sender (remote)
		struct sockaddr_in sinRemote;
		memset(&sinRemote, 0, sizeof(sinRemote));
		unsigned int sin_len = sizeof(sinRemote);

        char received[MSG_MAX_LEN];

		if ( recvfrom(socketDescriptor, received, MSG_MAX_LEN, 0, (struct sockaddr*) &sinRemote, &sin_len) < 0 ) {
			perror("writing to socket failed\n");
			exit(EXIT_FAILURE);
		}
		// entering critical section
		pthread_mutex_lock(s_pmutex);
        {
            // add received item to list to print
            List_add(pReceiveList, received);
        }
        pthread_mutex_unlock(s_pmutex);
		// done critical section

        // now wake up print thread
        pthread_mutex_lock(s_pmutex);
        {
            pthread_cond_signal(s_pOkToPrint);
        }
        pthread_mutex_unlock(s_pmutex);
        
	}
	return NULL;
}

void* printThread() {
    // wait for signal to print
    pthread_mutex_lock(s_pmutex);
		{
			pthread_cond_wait(s_pOkToPrint, s_pmutex);
		}
    pthread_mutex_unlock(s_pmutex);

	char toPrint[MSG_MAX_LEN];

    pthread_mutex_lock(s_pmutex);
		{
			// take item off list and store in string
			toPrint = List_remove(s_pPrintList);
		}
    pthread_mutex_unlock(s_pmutex);

	printf("%s\n",toPrint);
	return NULL;
}

void printThread_init(pthread_mutex_t *pmutex, pthread_cond_t *pOkToPrint, List* pPrintList)
{
    // store the parameters in the pointers that were initialized at the beginning
    s_pmutex = pmutex;
    s_pOkToPrint = pOkToPrint;
    s_pPrintList = pPrintList;

    pthread_create(&threadPrint, NULL, printThread, NULL);
}

void printThread_shutdown()
{
    pthread_join(threadPrint,NULL);
}

void receiveThread_init(pthread_mutex_t *pmutex, pthread_cond_t *pOkToPrint, List* pPrintList)
{
    // store the parameters in the pointers that were init at the beginning
    // of this file
    s_pmutex = pmutex;
    s_pOkToPrint = pOkToPrint;
    s_pPrintList = pPrintList;

    pthread_create(&threadReceive, NULL, receiveThread, NULL);
}

void receiveThread_shutdown()
{
    pthread_join(threadReceive,NULL);
}