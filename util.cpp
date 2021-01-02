#include "util.hpp"

bool addressParse(string address, string port, sockaddr_storage *storage) {
  uint16_t portNum = (uint16_t)atoi(&port[0]);
  if (portNum == 0) {
    return false;
  }
  portNum = htons(portNum); // host to network short

  struct in_addr inaddr4;
  if (inet_pton(AF_INET, &address[0], &inaddr4)) {
    struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
    addr4->sin_family = AF_INET;
    addr4->sin_port = portNum;
    addr4->sin_addr = inaddr4;
    return true;
  }

  struct in6_addr inaddr6;
  if (inet_pton(AF_INET6, &address[0], &inaddr6)) {
    struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
    addr6->sin6_family = AF_INET6;
    addr6->sin6_port = portNum;
    memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));
    return true;
  }

  return false;
}

AddressData getAddressData(sockaddr *addr) {
  
  int version;
  string addrstr;
  uint16_t port;

  if (addr->sa_family == AF_INET) {
    version = 4;
    struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
    if (!inet_ntop(AF_INET, &(addr4->sin_addr), &addrstr[0], INET6_ADDRSTRLEN + 1)) {
      exit(EXIT_FAILURE);
    }
    port = ntohs(addr4->sin_port); // network to host short
  
  } else if (addr->sa_family == AF_INET6) {
    version = 6;
    struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
    if (!inet_ntop(AF_INET6, &(addr6->sin6_addr), &addrstr[0], INET6_ADDRSTRLEN + 1)) {
      exit(EXIT_FAILURE);
    }
    port = ntohs(addr6->sin6_port); // network to host short
  
  } else {
    exit(EXIT_FAILURE);
  }

  AddressData addressData = AddressData(version, addrstr, port);
  return addressData;

}

bool initServerSockaddr(string proto, string portstr, sockaddr_storage *storage) {
  uint16_t port = (uint16_t)atoi(&portstr[0]); // unsigned short
  if (port == 0) {
   return false;
  }
  port = htons(port); // host to network short

  memset(storage, 0, sizeof(*storage));
  if (proto == "v4") {
    struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
    addr4->sin_family = AF_INET;
    addr4->sin_addr.s_addr = INADDR_ANY;
    addr4->sin_port = port;
    return true;

  } else if (proto == "v6") {
    struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
    addr6->sin6_family = AF_INET6;
    addr6->sin6_addr = in6addr_any;
    addr6->sin6_port = port;
    return true;
  
  } else {
    return false;
  
  }
}