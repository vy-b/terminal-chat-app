#ifndef _THREADS_H_
#define _THREADS_H_

#include <pthread.h>
#include "list.h"

void inputThread_init();
void inputThread_shutdown();

void sendThread_init();
void sendThread_shutdown();

void printThread_init();
void printThread_shutdown();

void receiveThread_init();
void receiveThread_shutdown();

void variables_init(List* pPrintList,
	List* pSendList, int* socketDescriptor, char* pRemoteHostName, 
	int* pportNumber);
#endif