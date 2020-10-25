/* includes input thread and send thread
input thread waits for keyboard input then adds to the list adt
signals send thread when input has been added and ready to send
*/
#include "shutdownmanager.h"
#include "input-send.h"
#include "list.h"
#include "helper.h"
#include <pthread.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MSG_MAX_LEN 1024

//defined in s-talk.c, passed here as pointers
static pthread_cond_t *s_pOkToSend;
static pthread_cond_t *s_pOkToShutdown;
static pthread_mutex_t *s_pmutex;
static List* s_pSendList;

static char* s_pRemoteHostName;
static int* s_pportNumber;

static pthread_t threadInput;
static pthread_t threadSend;
static int* s_socket;
static char* s_pmsg = NULL;

//defined in s-talk.c, passed here as pointers
static pthread_cond_t *s_pOkToPrint;
static List *s_pPrintList;

static pthread_t threadPrint;
static pthread_t threadReceive;
static char* s_preceived = NULL;
static int flag = 0;
static int closedSocket = 0;

// waits for input from keyboard and adds to list
void* inputThread() {
	while (1) {
		s_pmsg = malloc(MSG_MAX_LEN);
		 fgets(s_pmsg, MSG_MAX_LEN , stdin);
		
		
		// start critical section
		pthread_mutex_lock(s_pmutex);
		{
			//add to list
			if (List_add(s_pSendList, s_pmsg) != 0) {
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
		if (strcmp("!\n", s_pmsg) == 0)
		{
			printf("thread cancelling during input\n");
			
			ShutdownManager_waitForShutdown(s_pOkToShutdown, s_pmutex);
			if (s_pmsg) 
				free(s_pmsg);
			printf("input self shut down returns %d\n",ShutdownManager_isShuttingDown(pthread_self()));
		}
	}
}


// takes item off sendlist and send
void* sendThread() {
	struct hostent *remoteHost = gethostbyname(s_pRemoteHostName);
	if (remoteHost == NULL) 
	{
		perror("addr not found\n");
		exit(EXIT_FAILURE);
	}
	while (1) 
	{
		//after creating sockets, wait for signal to send (wait for item to be added to list)
		pthread_mutex_lock(s_pmutex);
		{
			pthread_cond_wait(s_pOkToSend, s_pmutex);
		}
		pthread_mutex_unlock(s_pmutex);

		char* toSend = NULL;

		pthread_mutex_lock(s_pmutex);
		{
			// take item off list and store in string
			toSend = List_remove(s_pSendList);
			
		}
		pthread_mutex_unlock(s_pmutex);
		
		//socket address of receiver (remote address)
		struct sockaddr_in sinRemote;
		memset(&sinRemote, 0, sizeof(sinRemote));
		sinRemote.sin_family = AF_INET; //IPv4 - don't need to implement IPv6
		memcpy(&sinRemote.sin_addr, remoteHost->h_addr_list[0], remoteHost->h_length);
		// sinRemote.sin_addr.s_addr = INADDR_ANY;
		sinRemote.sin_port = htons(*s_pportNumber);

		//------------for debugging---------------
		// char buffer[INET_ADDRSTRLEN];
		// inet_ntop( AF_INET, &sinRemote.sin_addr, buffer, sizeof( buffer ));
		// printf( "send address:%s\n", buffer );
		//------------for debugging---------------

		if ( sendto(*s_socket, toSend, MSG_MAX_LEN, 0, (struct sockaddr*) &sinRemote, sizeof(sinRemote)) < 0 ) {
			perror("writing to socket failed\n");
			exit(EXIT_FAILURE);
		}
		
		if (strcmp("!\n", toSend) == 0)
		{
			ShutdownManager_triggerShutdown(s_pOkToShutdown, s_pmutex);
			pthread_mutex_lock(s_pmutex);
			{
			if (closedSocket == 0){
				printf("closing socket from send\n");
				if (close(*s_socket)!=0)
				{
					perror("failed to close socket\n");
					exit(EXIT_FAILURE);
				}
				closedSocket = 1;
			}
			}
			pthread_mutex_unlock(s_pmutex);
			// this block is necessary to exit mutually if a ! is sent
			printf("receive shutdown from send %d\n", ShutdownManager_isShuttingDown(threadReceive));
			printf("print shutdown from send %d\n", ShutdownManager_isShuttingDown(threadPrint));
			//------------------------------------------------------------
			printf("send shutdown returns %d\n",ShutdownManager_isShuttingDown( pthread_self() ));
			if (toSend) {
				free(toSend);
				toSend = NULL;
			}
		}
		free(toSend);
	}
	return NULL;
}

void* receiveThread() {
    while (1) {
        // socket address of sender (remote)
        struct sockaddr_in sinRemote;
        memset(&sinRemote, 0, sizeof(sinRemote));
        unsigned int sin_len = sizeof(sinRemote);

        s_preceived = malloc(MSG_MAX_LEN);

        if ( recvfrom(*s_socket, s_preceived, MSG_MAX_LEN, 0, (struct sockaddr*) &sinRemote, &sin_len) < 0 ) {
            perror("receiving socket failed\n");
            exit(EXIT_FAILURE);
        }
        // entering critical section
        pthread_mutex_lock(s_pmutex);
        {

            // add received item to list to print
            List_add(s_pPrintList, s_preceived);

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
        if (strcmp("!\n", s_preceived) == 0)
		{
			ShutdownManager_waitForShutdown(s_pOkToShutdown, s_pmutex);
			printf("receive self shut down returns %d\n",ShutdownManager_isShuttingDown( pthread_self() ));
		}
    }
}

void* printThread() {
    while (1) {
        // wait for signal to print
        pthread_mutex_lock(s_pmutex);
        {
            while (flag == 0)
                pthread_cond_wait(s_pOkToPrint, s_pmutex);
        }
        pthread_mutex_unlock(s_pmutex);

        char* toPrint = NULL;

        pthread_mutex_lock(s_pmutex);
        {
            // take item off list and store in string
            toPrint = List_remove(s_pPrintList);
            flag = 0;
        }
        
        pthread_mutex_unlock(s_pmutex);
        if (strcmp("!\n", toPrint) == 0)
		{
			if (toPrint) {
				free(toPrint);
				toPrint = NULL;
			}

			pthread_mutex_lock(s_pmutex);
			{
			if (closedSocket == 0){
				printf("closing socket from print\n");
				if (close(*s_socket)!=0)
				{
					perror("failed to close socket\n");
					exit(EXIT_FAILURE);
				}
				closedSocket = 1;
			}
			}
			pthread_mutex_unlock(s_pmutex);


			// this block is necessary to exit mutually if a ! is received
			printf("input shutdown from print %d\n", ShutdownManager_isShuttingDown(threadInput));
			printf("send shutdown from print %d\n", ShutdownManager_isShuttingDown(threadSend));
			//------------------------------------------------------------
			printf("print self shut down returns %d\n",ShutdownManager_isShuttingDown( pthread_self() ));
		}

        if (toPrint) {
            printf("received: ");
            fputs(toPrint, stdout);
            free(toPrint);
        }
    }
}

void inputThread_init()
{
	pthread_create(&threadInput, NULL, inputThread, NULL);
}

void inputThread_shutdown()
{
	//pthread_cancel(threadInput);
	pthread_join(threadInput, NULL);
}

void sendThread_init(pthread_mutex_t *pmutex, pthread_cond_t *pOkToSend, List* pSendList, int* socketDescriptor, char* pRemoteHostAddr, int* pportNumber, int* pRemoteHostSize)
{
	pthread_create(&threadSend, NULL, sendThread, NULL);
}

void sendThread_shutdown()
{
	//pthread_cancel(threadSend);
	pthread_join(threadSend, NULL);
}

void sendVariables_init(pthread_mutex_t *pmutex, pthread_cond_t *pOkToSend,
	List* pSendList, int* socketDescriptor, char* pRemoteHostName, 
	int* pportNumber, pthread_cond_t *pOkToShutdown) 
{
	// store the parameters in the pointers that were init at the beginning
	// of this file
	s_pmutex = pmutex;
	s_pOkToSend = pOkToSend;
	s_pSendList = pSendList;
	s_socket = socketDescriptor;
	s_pRemoteHostName = pRemoteHostName;
	s_pportNumber = pportNumber;
	s_pOkToShutdown = pOkToShutdown;
}

void printThread_init()
{
    pthread_create(&threadPrint, NULL, printThread, NULL);
}

void printThread_shutdown()
{
    pthread_join(threadPrint, NULL);
}

void receiveThread_init(pthread_mutex_t *pmutex, pthread_cond_t *pOkToPrint, List* pPrintList, int* socketDescriptor)
{
    pthread_create(&threadReceive, NULL, receiveThread, NULL);
}

void receiveThread_shutdown()
{
    pthread_join(threadReceive, NULL);
}

void receiveVariables_init(pthread_mutex_t *pmutex, pthread_cond_t *pOkToPrint,
                           List* pPrintList, int* socketDescriptor, pthread_cond_t *pOkToShutdown) {
    // store the parameters in the pointers that were init at the beginning
    // of this file
    s_pmutex = pmutex;
    s_pOkToPrint = pOkToPrint;
    s_pPrintList = pPrintList;
    s_socket = socketDescriptor;
    s_pOkToShutdown = pOkToShutdown;
}