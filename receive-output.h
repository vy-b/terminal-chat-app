#ifndef _RECEIVE_OUTPUT_H
#define _RECEIVE_OUTPUT_H

#include <pthread.h>
#include "list.h"

void printThread_init(pthread_mutex_t *pmutex, pthread_cond_t *pOkToPrint, List* pPrintList);
void printThread_shutdown();

void receiveThread_init(pthread_mutex_t *pmutex, pthread_cond_t *pOkToPrint, List* pPrintList);
void receiveThread_shutdown();

#endif