#include <bits/stdc++.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include "util.hpp"

using namespace std;

#define BUFSZ 1024

void usage() {
  cout << "usage: ./server <v4|v6> <server port>" << endl;
  cout << "example: ./server v4 51511" << endl;
  exit(EXIT_FAILURE);
}

void validateArgs(int argc, char **argv) {
  if (argc < 3) {
    usage();
  }
}

SocketData createServer(string proto, string portStr) {
  struct sockaddr_storage storage;
  if (!initServerSockaddr(proto, portStr, &storage)) {
    usage();
  }
  
  int newSocket = socket(storage.ss_family, SOCK_STREAM, 0);
  if (newSocket == -1) {
    perror("Error while creating socket");
	  exit(EXIT_FAILURE);
  }

  int enable = 1;
  if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) != 0) {
    perror("Error in setsockopt");
	  exit(EXIT_FAILURE);
  }

  struct sockaddr *addressData = (struct sockaddr *)(&storage);
  if (bind(newSocket, addressData, sizeof(storage)) != 0) {
    perror("Error while binding");
	  exit(EXIT_FAILURE);
  }

  if (listen(newSocket, 10) != 0) {
    perror("Error while listening");
	  exit(EXIT_FAILURE);
  }

  AddressData addrData = getAddressData(addressData);
  SocketData serverData = SocketData(newSocket, storage, &addrData);
  
  return serverData;
}

void sendMessage(SocketData  *cliData, char *message) {
  size_t bufferLength = send(cliData->clientSocket, message, strlen(message) + 1, 0);
  if (bufferLength != strlen(message) + 1) {
    perror("Error while sending message from server");
    exit(EXIT_FAILURE);
  }
}

void* clientThread(void *data) {
  SocketData *cliData = (SocketData *)data;

  while (1) {
    char rcvMsgBuffer[BUFSZ];
    memset(rcvMsgBuffer, 0, BUFSZ);
    size_t bufferLength = recv(cliData->clientSocket, rcvMsgBuffer, BUFSZ - 1, 0);

    printf("[msg], %d bytes: %s\n", (int)bufferLength, rcvMsgBuffer);
    
    string msg = "Helloo Worldd!";
    sendMessage(cliData, &msg[0]);

  }

  close(cliData->clientSocket);
  pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {

  validateArgs(argc, argv);
  SocketData serverData = createServer(argv[1], argv[2]);

  while(1) {
    struct sockaddr_storage clientStorage;
    struct sockaddr *cliAddressData = (struct sockaddr *)(&clientStorage);
    socklen_t cliAdrrLength = sizeof(clientStorage);

    int cliSocket = accept(serverData.clientSocket, cliAddressData, &cliAdrrLength);
    if (cliSocket == -1) {
      perror("Error while accepting client connection");
	    exit(EXIT_FAILURE);
    }

    AddressData addrData = getAddressData(cliAddressData);    
    SocketData cliData = SocketData(cliSocket, clientStorage, &addrData);

    pthread_t tid;
    pthread_create(&tid, NULL, clientThread, &cliData);
  
  }


  return 0;
}
