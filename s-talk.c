
#include <stdio.h>
#include <stdlib.h>

#include "input-send.h"
#include "receive-output.h"
#include "list.h"

// initialize everything here, pass in other files
static pthread_cond_t s_OkToSend = PTHREAD_COND_INITIALIZER;
static pthread_cond_t s_OkToPrint = PTHREAD_COND_INITIALIZER;

static pthread_mutex_t s_mutex = PTHREAD_MUTEX_INITIALIZER;

//CONVERT HOSTNAME TO IP
//struct hostent *hp = gethostbyname(argv[whatever postion the hostname is in]);
//memcpy(&sin.sin_addr.s_addr, hp->h_addr_list[0], hp->h_length);

int main() {
    List* SendList = List_create();
    List* PrintList = List_create();

	inputThread_init(&s_mutex, &s_OkToSend, SendList);
	sendThread_init(&s_mutex, &s_OkToSend, SendList);
	receiveThread_init(&s_mutex, &s_OkToPrint, PrintList);
	printThread_init(&s_mutex,&s_OkToPrint, PrintList);


	sendThread_shutdown();
	inputThread_shutdown();
	receiveThread_shutdown();
	printThread_shutdown();
}
