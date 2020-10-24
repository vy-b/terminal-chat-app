#ifndef _INPUT_RECEIVE_H_
#define _INPUT_RECEIVE_H_

#include <pthread.h>
#include "list.h"

void inputThread_init();
void inputThread_shutdown();

void sendThread_init();
void sendThread_shutdown();

void sendVariables_init(pthread_mutex_t *pmutex, pthread_cond_t *pOkToSend, 
    List* pSendList, int* socketDescriptor, char*  pRemoteHostName, int* pportNumber, pthread_cond_t *pOkToShutdown);
void printThread_init();
void printThread_shutdown();

void receiveThread_init();
void receiveThread_shutdown();

void receiveVariables_init(pthread_mutex_t *pmutex, pthread_cond_t *pOkToPrint, 
    List* pPrintList, int* socketDescriptor, pthread_cond_t *pOkToShutdown);
#endif