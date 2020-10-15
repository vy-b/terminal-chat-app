

all:
	gcc -Wall -Werror s-talk.c list.o helper.c input-send.c receive-output.c -o s-talk -lpthread


clean:
	rm s-talk