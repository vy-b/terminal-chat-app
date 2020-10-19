#ifndef _INPUT_RECEIVE_H_
#define _INPUT_RECEIVE_H_

#include <pthread.h>
#include "list.h"

void inputThread_init();
void inputThread_shutdown();

void sendThread_init();
void sendThread_shutdown();

void sendVariables_init(pthread_mutex_t *pmutex, pthread_cond_t *pOkToSend, 
    List* pSendList, int* socketDescriptor, char* pRemoteHostName, int* pportNumber);

#endif