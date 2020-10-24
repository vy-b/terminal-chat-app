#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include "helper.h"
#include "input-send.h"
#include "receive-output.h"
#include "list.h"
#include "shutdownmanager.h"


static pthread_cond_t *s_pOkToShutdown;
static pthread_mutex_t *s_pmutex;

void ShutdownManager_waitForShutdown(pthread_t thread)
{
	pthread_mutex_lock(s_pmutex);
        {
            pthread_cond_wait(s_pOkToShutdown, s_pmutex);
            
            pthread_cancel(thread);
   			pthread_join(thread, NULL);
        }
    pthread_mutex_unlock(s_pmutex);
}

void ShutdownManager_triggerShutdown(pthread_t thread)
{
	pthread_cancel(thread);
   	pthread_join(thread, NULL);

	// pthread_mutex_lock(s_pmutex);
	// 	{
	// 		pthread_cond_signal(s_pOkToShutdown);
	// 	}
	// pthread_mutex_unlock(s_pmutex);
}

bool ShutdownManager_isShuttingDown();