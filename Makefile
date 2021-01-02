all:
	g++ -c util.cpp -o util
	g++ client.cpp util.o -lpthread -o client
	g++ server.cpp util.o -lpthread -o server

clean:
	rm util.o client server