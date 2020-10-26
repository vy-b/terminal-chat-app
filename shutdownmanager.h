#ifndef _SHUTDOWN_MANAGER_H
#define _SHUTDOWN_MANAGER_H

#include <pthread.h>
void ShutdownManager_waitForShutdown(pthread_cond_t *pOkToShutdown,pthread_mutex_t *pmutex);
void ShutdownManager_triggerShutdown(pthread_cond_t *pOkToShutdown);
int ShutdownManager_isShuttingDown(pthread_t thread);
void free_mutexes(void *mutex);
void free_cond(void* cond);
void free_malloc(void* item);
#endif