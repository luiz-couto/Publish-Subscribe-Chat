#include <bits/stdc++.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include "util.hpp"

using namespace std;

#define BUFSZ 1024

void usage() {
  cout << "usage: ./server <server IP> <server port>" << endl;
	cout << "example: ./server 127.0.0.1 51511" << endl;
	exit(EXIT_FAILURE);
}

void validateArgs(int argc, char **argv) {
  if (argc < 3) {
    usage();
  }
}

SocketData createSocket(string addr, string port) {
  struct sockaddr_storage storage;
  if (!addressParse(addr, port, &storage)) {
    usage();
  }

  int newSocket = socket(storage.ss_family, SOCK_STREAM, 0);
  if (newSocket == -1) {
    perror("Error while creating socket");
	  exit(EXIT_FAILURE);
  }

  struct sockaddr *addressData = (struct sockaddr *)(&storage);
  if (connect(newSocket, addressData, sizeof(storage)) != 0) {
    perror("Error while opening a socket connection");
	  exit(EXIT_FAILURE);
  }

  AddressData addrData = getAddressData(addressData);

  SocketData clientData = SocketData(newSocket, storage, &addrData);
  return clientData;
}

bool sendMessage(SocketData *clientData, string message) {
  size_t len = send(clientData->clientSocket, &message[0], strlen(&message[0])+1, 0);
	if (len != strlen(&message[0])+1) {
		cout << "Error while sending message" << endl;
    return false;
	}
  return true;
}

void * sendThread(void *data) {
  SocketData *clientData = (SocketData *)data;
  while(1) {
    cout << "> ";
    string message;
    getline(cin, message);
    if (!sendMessage(clientData, message)) {
      break;
    }
  }
  pthread_exit(NULL);
}

void * receiveThread(void *data) {
  SocketData *clientData = (SocketData *)data;

  while (1) {
    char rcvMsgBuffer[BUFSZ];
    memset(rcvMsgBuffer, 0, BUFSZ);
    size_t bufferLength = recv(clientData->clientSocket, rcvMsgBuffer, BUFSZ - 1, 0);

    printf("[msg], %d bytes: %s\n", (int)bufferLength, rcvMsgBuffer);

  }

  pthread_exit(NULL);
}

int main(int argc, char **argv) {
  validateArgs(argc, argv);
  SocketData clientData = createSocket(argv[1], argv[2]);

  pthread_t tid2;
  pthread_create(&tid2, NULL, receiveThread, &clientData);

  sendThread(&clientData);

  sendMessage(&clientData, "Hello World!");
  close(clientData.clientSocket);

  return 0;
}