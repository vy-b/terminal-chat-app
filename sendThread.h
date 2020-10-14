#ifndef _SENDTHREAD_H_
#define _SENDTHREAD_H_

#include <pthread.h>
#define MSG_MAX_LEN 1024
#define PORT 22110

void sendThread_init(pthread_mutex_t *pmutex, pthread_cond_t *pOkToSend, List* pSendList);
void sendThread_shutdown();