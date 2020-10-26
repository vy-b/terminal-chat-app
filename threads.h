#ifndef _THREADS_H_
#define _THREADS_H_

#include <pthread.h>
#include "list.h"

void inputThread_init();
int inputThread_shutdown();

void sendThread_init();
int sendThread_shutdown();

void printThread_init();
int printThread_shutdown();

void receiveThread_init();
int receiveThread_shutdown();

void variables_init(List* pPrintList,
	List* pSendList, int* socketDescriptor, char* pRemoteHostName, 
	int* pportNumber);


#endif