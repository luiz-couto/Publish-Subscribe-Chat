all:
	g++ -c util.cpp
	g++ client.cpp util.o -lpthread -o cliente
	g++ server.cpp util.o -lpthread -o servidor

clean:
	rm util.o client server util