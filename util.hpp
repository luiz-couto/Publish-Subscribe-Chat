#ifndef UTIL_H
#define UTIL_H

#include <bits/stdc++.h>
#include <arpa/inet.h>

using namespace std;

struct AddressData {
  int version;
  string addressStr;
  uint16_t port;
  AddressData(int version, string addressStr, uint16_t port) {
    this->version = version;
    this->addressStr = addressStr;
    this->port = port;
  }
};

struct SocketData {
  int clientSocket;
  struct sockaddr_storage storage;
  AddressData *addressData;
  SocketData(int clientSocket, sockaddr_storage storage, AddressData *addressData) {
    this->clientSocket = clientSocket;
    this->storage = storage;
    this->addressData = addressData;
  }
};

struct ClientData {
  SocketData *clientData;
  vector<string> subscriptions;
  ClientData (SocketData *clientData) {
    this->clientData = clientData;
    this->subscriptions = {};
  }
};

struct ServerData {
  vector<ClientData *> clients;
  ServerData() {
    this->clients = {};
  }
};

bool addressParse(string address, string port, sockaddr_storage *storage);
AddressData getAddressData(sockaddr *addr);
bool initServerSockaddr(string proto, string portstr, sockaddr_storage *storage);


#endif