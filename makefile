

all:
	gcc -Wall -Werror s-talk.c list.o helper.c input-send.c receive-output.c -o s-talk -lpthread

valgrind: build
	valgrind --leak-check=full ./s-talk

clean:
	rm s-talk