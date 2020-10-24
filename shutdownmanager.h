#ifndef _SHUTDOWN_MANAGER_H
#define _SHUTDOWN_MANAGER_H

#include <pthread.h>

void ShutdownManager_triggerShutdown(pthread_t thread);

#endif