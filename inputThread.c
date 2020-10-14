#include "inputThread.h"

//defined in s-talk.c, passed here as pointers
static pthread_cond_t *s_pOkToSend;
static pthread_mutex_t *s_pmutex;
static *s_pSendList;

pthread_t threadInput;

// waits for input from keyboard and adds to list
void* inputThread(){
	while (1){
		char* msg[MSG_MAX_LEN];
		fgets(msg, MSG_MAX_LEN,stdin);

		// start critical section
		pthread_mutex_lock(&s_pmutex);
		{
			//add to list
			List_add(s_pSendList, msg);
		}
		pthread_mutex_unlock(&s_pmutex);
		// end critical section
		
		// now wake up send thread
		pthread_mutex_lock(&s_pmutex);
		{
			pthread_cond_signal(s_pOkToSend);
		}
		pthread_mutex_unlock(&s_pmutex);
	}
}

void inputThread_init(pthread_mutex_t *pmutex, pthread_cond_t *pOkToSend, List* pSendList)
{
    // store the parameters in the pointers that were init at the beginning
    // of this file
    s_pmutex = pmutex;
    s_pOkToSend = pOkToSend;
    s_pSendList = pSendList;

    pthread_create(&threadInput, NULL, inputThread, NULL);
}

void inputThread_shutdown()
{
    pthread_join(threadInput,NULL);
}