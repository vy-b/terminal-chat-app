#ifndef _RECEIVE_OUTPUT_H
#define _RECEIVE_OUTPUT_H

#include <pthread.h>
#include "list.h"

void printThread_init();
void printThread_shutdown();

void receiveThread_init();
void receiveThread_shutdown();

void receiveVariables_init(pthread_mutex_t *pmutex, pthread_cond_t *pOkToPrint, 
    List* pPrintList, int* socketDescriptor, pthread_cond_t *pOkToShutdown);
#endif