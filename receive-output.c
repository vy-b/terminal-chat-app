#include "receive-output.h"
#include "list.h"
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
int* s_socket;

void* receiveThread(){
	while(1){
		// socket address of sender (remote)
		struct sockaddr_in sinRemote;
		memset(&sinRemote, 0, sizeof(sinRemote));
        sinRemote.sin_family = AF_INET; //IPv4 - don't need to implement IPv6
		sinRemote.sin_addr.s_addr = INADDR_ANY;
		sinRemote.sin_port = htons(PORT);
		unsigned int sin_len = sizeof(sinRemote);

        char received[MSG_MAX_LEN];

		if ( recvfrom(*s_socket, received, MSG_MAX_LEN, 0, (struct sockaddr*) &sinRemote, &sin_len) < 0 ) {
			perror("receiving socket failed\n");
			exit(EXIT_FAILURE);
		}
		// entering critical section
		pthread_mutex_lock(s_pmutex);
        {
            // add received item to list to print
            List_add(s_pPrintList, received);
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
    while (1){
        // wait for signal to print
        pthread_mutex_lock(s_pmutex);
            {
                pthread_cond_wait(s_pOkToPrint, s_pmutex);
            }
        pthread_mutex_unlock(s_pmutex);

        char* toPrint;

        pthread_mutex_lock(s_pmutex);
            {
                // take item off list and store in string
                toPrint = List_remove(s_pPrintList);
            }
        pthread_mutex_unlock(s_pmutex);

        printf("%s\n",toPrint);
        printf("received.\n");
        return NULL;
    }
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

void receiveThread_init(pthread_mutex_t *pmutex, pthread_cond_t *pOkToPrint, List* pPrintList, int* socketDescriptor)
{
    // store the parameters in the pointers that were init at the beginning
    // of this file
    s_pmutex = pmutex;
    s_pOkToPrint = pOkToPrint;
    s_pPrintList = pPrintList;
    s_socket = socketDescriptor;

    pthread_create(&threadReceive, NULL, receiveThread, NULL);
}

void receiveThread_shutdown()
{
    pthread_join(threadReceive,NULL);
}