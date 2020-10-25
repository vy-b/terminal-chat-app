#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <netdb.h>
#include "shutdownmanager.h"




void ShutdownManager_waitForShutdown(pthread_cond_t *pOkToShutdown, pthread_mutex_t *pShutdownMutex)
{
	pthread_mutex_lock(pShutdownMutex);
	{
		pthread_cond_wait(pOkToShutdown, pShutdownMutex);
	}
    pthread_mutex_unlock(pShutdownMutex);
}

void ShutdownManager_triggerShutdown(pthread_cond_t *pOkToShutdown, pthread_mutex_t *pShutdownMutex)
{
	pthread_cond_signal(pOkToShutdown);
	

	// pthread_mutex_lock(s_pmutex);
	// 	{
	// 		pthread_cond_signal(s_pOkToShutdown);
	// 	}
	// pthread_mutex_unlock(s_pmutex);
}

int ShutdownManager_isShuttingDown(pthread_t thread){
	return pthread_cancel(thread);
}