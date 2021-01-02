#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <sys/types.h>

#include "util.h"

#define BUFSZ 1024

void usage(char **argv) {
  printf("usage: %s <v4|v6> <server port>\n", argv[0]);
  printf("example: %s v4 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}

struct clientData {
  int csock;
  struct sockaddr_storage storage;
};

void* clientThread(void *data) {
  struct clientData *cliData = (struct clientData *)data;
  //struct sockaddr *cliAddressData = (struct sockaddr *)(&cliData->storage);

  char rcvMsgBuffer[BUFSZ];
  memset(rcvMsgBuffer, 0, BUFSZ);
  size_t bufferLength = recv(cliData->csock, rcvMsgBuffer, BUFSZ - 1, 0);

  printf("[msg], %d bytes: %s\n", (int)bufferLength, rcvMsgBuffer);

  sprintf(rcvMsgBuffer, "remote endpoint: - \n");
  bufferLength = send(cliData->csock, rcvMsgBuffer, strlen(rcvMsgBuffer) + 1, 0);
  if (bufferLength != strlen(rcvMsgBuffer) + 1) {
    perror("Error while sending message from server");
	  exit(EXIT_FAILURE);
  }

  close(cliData->csock);
  pthread_exit(EXIT_SUCCESS);


}

int main(int argc, char **argv) {
  if (argc < 3) {
    usage(argv);
  }

  struct sockaddr_storage storage;
  if (0 != initServerSockaddr(argv[1], argv[2], &storage)) {
    usage(argv);
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

  while(1) {
    struct sockaddr_storage clientStorage;
    struct sockaddr *cliAddressData = (struct sockaddr *)(&clientStorage);
    socklen_t cliAdrrLength = sizeof(clientStorage);

    int cliSocket = accept(newSocket, cliAddressData, &cliAdrrLength);
    if (cliSocket == -1) {
      perror("Error while accepting client connection");
	    exit(EXIT_FAILURE);
    }

    struct clientData *cliData = malloc(sizeof(*cliData));
    if (!cliData) {
      perror("Error in memory allocation for client data");
	    exit(EXIT_FAILURE);
    }

    cliData->csock = cliSocket;
    memcpy(&(cliData->storage), &clientStorage, sizeof(clientStorage));

    pthread_t tid;
    pthread_create(&tid, NULL, clientThread, cliData);
  
  }

  exit(EXIT_SUCCESS);

}