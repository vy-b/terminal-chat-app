
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include "helper.h"
#include "input-send.h"
#include "list.h"
#include "shutdownmanager.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int main(int argc, char* argv[]) {
    List* SendList = List_create();
    List* PrintList = List_create();

	// checks if there are 4 arguments when the program is initialized in terminal
	if (argc != 4) {
	    perror("usage: ./s-talk myPort RemoteMachineName RemotePort\n");
	    exit(EXIT_FAILURE);
	}
	char* portName = argv[3];
	int portNumber = atoi(portName);
	
	char* myPortName = argv[1];
	int myPortNumber = atoi(myPortName);

	char* remoteHostName = argv[2];
	

	// for debugging----------------------------------------
	
	printf("myPortNumber: %d\n",myPortNumber);
	printf("remote port number: %d\n", portNumber);
	printf("remoteHostName: %s\n", argv[2]);
	// for debugging --------------------------------------------

	int socketDescriptor = socket_init(&myPortNumber);

	variables_init(PrintList,SendList, &socketDescriptor, remoteHostName, &portNumber);


	inputThread_init();
	sendThread_init();
	receiveThread_init();
	printThread_init();


	sendThread_shutdown();
	inputThread_shutdown();
	receiveThread_shutdown();
	printThread_shutdown();
}
