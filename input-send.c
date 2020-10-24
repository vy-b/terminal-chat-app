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
static pthread_mutex_t *s_pmutex;
static List* s_pSendList;

static char* s_pRemoteHostName;
static int* s_pportNumber;

pthread_t threadInput;
pthread_t threadSend;
static int* s_socket;
static char* s_pmsg = NULL;

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

		if (strcmp("!\n", s_pmsg) == 0)
		{
			ShutdownManager_triggerShutdown(threadInput);
			
			if (s_pmsg) {
				free(s_pmsg);
				s_pmsg = NULL;
			}

		}
		// now wake up send thread
		pthread_mutex_lock(s_pmutex);
		{
			pthread_cond_signal(s_pOkToSend);
		}
		pthread_mutex_unlock(s_pmutex);
	}
}


// takes item off sendlist and send
void* sendThread() {
	struct hostent *remoteHost = gethostbyname(s_pRemoteHostName);
	if (remoteHost == NULL) {
		perror("addr not found\n");
		exit(EXIT_FAILURE);
	}
	while (1) {
		//after creating sockets, wait for signal to send (wait for item to be added to list)
		pthread_mutex_lock(s_pmutex);
		{
			pthread_cond_wait(s_pOkToSend, s_pmutex);
		}
		pthread_mutex_unlock(s_pmutex);

		char* toSend;

		pthread_mutex_lock(s_pmutex);
		{
			// take item off list and store in string
			toSend = List_remove(s_pSendList);
			free(s_pmsg);
		}
		pthread_mutex_unlock(s_pmutex);

		//socket address of receiver (remote address)
		struct sockaddr_in sinRemote;
		// memset(&sinRemote, 0, sizeof(sinRemote));
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

		if (strcmp("!\n", s_pmsg) == 0)
		{
			ShutdownManager_triggerShutdown(threadSend);
			if (s_pmsg) {
				free(s_pmsg);
				s_pmsg = NULL;
			}

		}
	}

	return NULL;
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
                        List* pSendList, int* socketDescriptor, char* pRemoteHostName, int* pportNumber) {
	// store the parameters in the pointers that were init at the beginning
	// of this file
	s_pmutex = pmutex;
	s_pOkToSend = pOkToSend;
	s_pSendList = pSendList;
	s_socket = socketDescriptor;
	s_pRemoteHostName = pRemoteHostName;
	s_pportNumber = pportNumber;
}