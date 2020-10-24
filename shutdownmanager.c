#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <netdb.h>
#include "shutdownmanager.h"




void ShutdownManager_waitForShutdown(pthread_cond_t *pOkToShutdown, pthread_mutex_t *pmutex)
{
	pthread_mutex_lock(pmutex);
	{
		pthread_cond_wait(pOkToShutdown, pmutex);
	}
    pthread_mutex_unlock(pmutex);
}

void ShutdownManager_triggerShutdown(pthread_cond_t *pOkToShutdown, pthread_mutex_t *pmutex)
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