all:
	gcc -Wall -c util.c
	gcc -Wall client.c util.o -o client
	gcc -Wall server.c util.o -lpthread -o server

clean:
	rm util.o client server server