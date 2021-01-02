#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <arpa/inet.h>

bool addressParse(const char *address, const char *port, struct sockaddr_storage *storage) {
  
  if (address == NULL || port == NULL) {
    return false;
  }

  uint16_t portNum = (uint16_t)atoi(port);
  if (portNum == 0) {
    return false;
  }
  portNum = htons(portNum); // host to network short

  struct in_addr inaddr4;
  if (inet_pton(AF_INET, address, &inaddr4)) {
    struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
    addr4->sin_family = AF_INET;
    addr4->sin_port = portNum;
    addr4->sin_addr = inaddr4;
    return true;
  }

  struct in6_addr inaddr6;
  if (inet_pton(AF_INET6, address, &inaddr6)) {
    struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
    addr6->sin6_family = AF_INET6;
    addr6->sin6_port = portNum;
    memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));
    return true;
  }

  return false;

}

int initServerSockaddr(const char *proto, const char *portstr, struct sockaddr_storage *storage) {
  uint16_t port = (uint16_t)atoi(portstr); // unsigned short
  if (port == 0) {
    return -1;
  }
  port = htons(port); // host to network short

  memset(storage, 0, sizeof(*storage));
  if (0 == strcmp(proto, "v4")) {
    struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
    addr4->sin_family = AF_INET;
    addr4->sin_addr.s_addr = INADDR_ANY;
    addr4->sin_port = port;
    return 0;
  } else if (0 == strcmp(proto, "v6")) {
    struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
    addr6->sin6_family = AF_INET6;
    addr6->sin6_addr = in6addr_any;
    addr6->sin6_port = port;
    return 0;
  } else {
    return -1;
  }
}