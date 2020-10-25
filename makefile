

all:
	gcc -Wall -Werror s-talk.c list.o socket.c threads.c shutdownmanager.c -o s-talk -lpthread

valgrind: build
	valgrind --leak-check=full ./s-talk

clean:
	rm s-talk