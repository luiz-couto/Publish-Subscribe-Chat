#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "util.h"

#define BUFSZ 1024

void usage(char **argv) {
  printf("usage: %s <server IP> <server port>\n", argv[0]);
	printf("example: %s 127.0.0.1 51511\n", argv[0]);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    usage(argv);
  }

  struct sockaddr_storage storage;
  if (!addressParse(argv[1], argv[2], &storage)) {
    usage(argv);
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

  char msgBuffer[BUFSZ];
  memset(msgBuffer, 0, BUFSZ);
  printf("> ");

  fgets(msgBuffer, BUFSZ-1, stdin);
  size_t bufferLength = send(newSocket, msgBuffer, strlen(msgBuffer) + 1, 0);
  if (bufferLength != strlen(msgBuffer)+1) {
    perror("Error while sending message");
	  exit(EXIT_FAILURE);
  }

  memset(msgBuffer, 0, BUFSZ);
  unsigned total = 0;
  while(1) {
    bufferLength = recv(newSocket, msgBuffer + total, BUFSZ - total, 0);
    if (bufferLength == 0) {
      break;
    }
    total = total + bufferLength;
  }

  close(newSocket);
  puts(msgBuffer);

  exit(EXIT_SUCCESS);

}

