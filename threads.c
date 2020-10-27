/* includes input thread and send thread
input thread waits for keyboard input then adds to the list adt
signals send thread when input has been added and ready to send
*/
#include "shutdownmanager.h"
#include "list.h"
#include <pthread.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>

#define MSG_MAX_LEN 1024

static pthread_mutex_t shutdownMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_OkToShutdown = PTHREAD_COND_INITIALIZER;

static pthread_cond_t s_OkToSend = PTHREAD_COND_INITIALIZER;
static pthread_cond_t s_OkToPrint = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t s_SharedListMutex = PTHREAD_MUTEX_INITIALIZER;

static List* s_pSendList;
static List* s_pPrintList;

static char* s_pRemoteHostName;
static int* s_pportNumber;

static pthread_t threadPrint;
static pthread_t threadReceive;
static pthread_t threadInput;
static pthread_t threadSend;

static int* s_socket;
static char* s_pmsg = NULL;
static char* s_preceived = NULL;
static int flag = 0;
static int closedSocket = 0;


// waits for input from keyboard and adds to list
void* inputThread() {
	while (1) {
		s_pmsg = malloc(MSG_MAX_LEN);
		fgets(s_pmsg, MSG_MAX_LEN , stdin);
		
		
		// start critical section
		pthread_mutex_lock(&s_SharedListMutex);
		{
			//add to list
			if (List_add(s_pSendList, s_pmsg) != 0) {
				perror("List add failed\n");
				exit(EXIT_FAILURE);
			}
		}
		pthread_mutex_unlock(&s_SharedListMutex);
		// end critical section

		// now wake up send thread
		pthread_mutex_lock(&s_SharedListMutex);
		{
			pthread_cond_signal(&s_OkToSend);
		}
		pthread_mutex_unlock(&s_SharedListMutex);

		if (strcmp("!\n", s_pmsg) == 0)
		{
			ShutdownManager_waitForShutdown(&s_OkToShutdown, &shutdownMutex);
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
		//wait for signal to send (wait for item to be added to list)
		pthread_mutex_lock(&s_SharedListMutex);
		{
			pthread_cond_wait(&s_OkToSend, &s_SharedListMutex);
		}
		pthread_mutex_unlock(&s_SharedListMutex);

		char* toSend = NULL;
		memset(&toSend, 0, sizeof(toSend));
		pthread_mutex_lock(&s_SharedListMutex);
		{
			// take item off list and store in string
			toSend = List_remove(s_pSendList);
		}
		pthread_mutex_unlock(&s_SharedListMutex);
		
		//socket address of receiver (remote address)
		struct sockaddr_in sinRemote;
		memset(&sinRemote, 0, sizeof(sinRemote));
		sinRemote.sin_family = AF_INET; //IPv4 - don't need to implement IPv6
		memcpy(&sinRemote.sin_addr, remoteHost->h_addr_list[0], remoteHost->h_length);
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
			if (toSend) {
				free(toSend);
			}
			ShutdownManager_triggerShutdown(&s_OkToShutdown);

			pthread_mutex_lock(&s_SharedListMutex);
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
			pthread_mutex_unlock(&s_SharedListMutex);
			
			pthread_cleanup_push(free_mutexes, &s_SharedListMutex);
			pthread_cleanup_push(free_mutexes, &shutdownMutex);
			pthread_cleanup_push(free_cond, &s_OkToSend);
			pthread_cleanup_push(free_cond, &s_OkToPrint);
			pthread_cleanup_push(free_cond, &s_OkToShutdown);

			// this block is necessary to exit mutually if a ! is sent but not received
			printf("receive shutdown from send %d\n", ShutdownManager_isShuttingDown(threadReceive));
			printf("print shutdown from send %d\n", ShutdownManager_isShuttingDown(threadPrint));
			//------------------------------------------------------------
			printf("send self shutdown returns %d\n",ShutdownManager_isShuttingDown( pthread_self() ));
			pthread_cleanup_pop(1);
			pthread_cleanup_pop(1);
			pthread_cleanup_pop(1);
			pthread_cleanup_pop(1);
			pthread_cleanup_pop(1);
		}
		if (toSend)
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
        pthread_mutex_lock(&s_SharedListMutex);
        {

            // add received item to list to print
            List_add(s_pPrintList, s_preceived);

        }
        pthread_mutex_unlock(&s_SharedListMutex);
        // done critical section

        
        // now wake up print thread
        pthread_mutex_lock(&s_SharedListMutex);
        {
            flag = 1;
            pthread_cond_signal(&s_OkToPrint);
        }
        pthread_mutex_unlock(&s_SharedListMutex);
        if (strcmp("!\n", s_preceived) == 0)
		{
			ShutdownManager_waitForShutdown(&s_OkToShutdown, &shutdownMutex);
			printf("receive self shut down returns %d\n",ShutdownManager_isShuttingDown( pthread_self() ));
		}
    }
}

void* printThread() {
    while (1) {
        // wait for signal to print
        pthread_mutex_lock(&s_SharedListMutex);
        {
            while (flag == 0)
                pthread_cond_wait(&s_OkToPrint, &s_SharedListMutex);
        }
        pthread_mutex_unlock(&s_SharedListMutex);

        char* toPrint = NULL;

        pthread_mutex_lock(&s_SharedListMutex);
        {
            // take item off list and store in string
            toPrint = List_remove(s_pPrintList);
            flag = 0;
        }
        
        pthread_mutex_unlock(&s_SharedListMutex);
        if (strcmp("!\n", toPrint) == 0)
		{
			if (toPrint) {
				free(toPrint);
			}
			// reusing the shared list mutex for socket closing (which is shared between send and receive)
			pthread_mutex_lock(&s_SharedListMutex);
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
			pthread_mutex_unlock(&s_SharedListMutex);

			ShutdownManager_triggerShutdown(&s_OkToShutdown);

			// this block is necessary to exit mutually if a ! is received but not sent
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

int inputThread_shutdown()
{
	//pthread_cancel(threadInput);
	return pthread_join(threadInput, NULL);
}

void sendThread_init()
{
	pthread_create(&threadSend, NULL, sendThread, NULL);
}

int sendThread_shutdown()
{
	//pthread_cancel(threadSend);
	return pthread_join(threadSend, NULL);
}

void variables_init(List* pPrintList,
	List* pSendList, int* socketDescriptor, char* pRemoteHostName, 
	int* pportNumber) 
{
	s_pSendList = pSendList;
	s_pPrintList = pPrintList;
	s_socket = socketDescriptor;
	s_pRemoteHostName = pRemoteHostName;
	s_pportNumber = pportNumber;
}

void printThread_init()
{
    pthread_create(&threadPrint, NULL, printThread, NULL);
}

int printThread_shutdown()
{
    return pthread_join(threadPrint, NULL);
}

void receiveThread_init()
{
    pthread_create(&threadReceive, NULL, receiveThread, NULL);
}

int receiveThread_shutdown()
{
    return pthread_join(threadReceive, NULL);
}

