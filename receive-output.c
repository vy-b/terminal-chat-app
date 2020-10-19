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

//defined in s-talk.c, passed here as pointers
static pthread_cond_t *s_pOkToPrint;
static pthread_mutex_t *s_pmutex;
static List *s_pPrintList;

pthread_t threadPrint;
pthread_t threadReceive;
static int* s_socket;

static int flag = 0;

void* receiveThread(){
	while(1){
		// socket address of sender (remote)
		struct sockaddr_in sinRemote;
		memset(&sinRemote, 0, sizeof(sinRemote));
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
            flag = 1;
            pthread_cond_signal(s_pOkToPrint);
        }
        pthread_mutex_unlock(s_pmutex);
        
	}
}

void* printThread() {
    while (1){
        // wait for signal to print
        pthread_mutex_lock(s_pmutex);
        {
            while (flag == 0)
                pthread_cond_wait(s_pOkToPrint, s_pmutex);
        }
        pthread_mutex_unlock(s_pmutex);
        
        char* toPrint;

        pthread_mutex_lock(s_pmutex);
        {
            // take item off list and store in string
            toPrint = List_remove(s_pPrintList);
            flag = 0;
        }
        pthread_mutex_unlock(s_pmutex);
        printf("received: ");
        fputs(toPrint,stdout);
    }
}

void printThread_init()
{
    pthread_create(&threadPrint, NULL, printThread, NULL);
}

void printThread_shutdown()
{
    pthread_join(threadPrint,NULL);
}

void receiveThread_init(pthread_mutex_t *pmutex, pthread_cond_t *pOkToPrint, List* pPrintList, int* socketDescriptor)
{
    pthread_create(&threadReceive, NULL, receiveThread, NULL);
}

void receiveThread_shutdown()
{
    pthread_join(threadReceive,NULL);
}

void receiveVariables_init(pthread_mutex_t *pmutex, pthread_cond_t *pOkToPrint, 
    List* pPrintList, int* socketDescriptor){
    // store the parameters in the pointers that were init at the beginning
    // of this file
    s_pmutex = pmutex;
    s_pOkToPrint = pOkToPrint;
    s_pPrintList = pPrintList;
    s_socket = socketDescriptor;

}
/* to do 
1. (just to clean up code): implement a receive-output initializer that allocates 
the pointers to the mutex, condition variables, socket descriptor and malloc'd buffers.

2. pthread_cancel on "!" input

3. take hostname in main (or encapsulate by calling a function to help?)*/