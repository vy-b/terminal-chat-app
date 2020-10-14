#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define MSG_MAX_LEN 1024
#define PORT 22110

void inputThread_init(pthread_mutex_t *pmutex, pthread_cond_t *pOkToSend, List* pSendList);
void inputThread_shutdown();