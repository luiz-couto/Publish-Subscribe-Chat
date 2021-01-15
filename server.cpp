#include <bits/stdc++.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include "util.hpp"

#define INF 1000000000
#define _ ios_base::sync_with_stdio(0);cin.tie(0);
#define rep(i, a, b) for(int i = int(a); i < int(b); i++)
#define debug(x) cout << #x << " = " << x << endl;
#define debug2(x,y) cout << #x << " = " << x << " --- " << #y << " = " << y << "\n";
#define debugA(x, l) { rep(i,0,l) { cout << x[i] << " "; } printf("\n"); }
#define debugM( x, l, c ) { rep( i, 0, l ){ rep( j, 0, c ) cout << x[i][j] << " "; printf("\n");}}
#define setM( x, l, c, k ) { rep( i, 0, l ){ rep( j, 0, c ) x[i][j] = k;}}

using namespace std;

#define BUFSZ 1024
#define INVALID -1
#define NORMAL 0
#define SUBSCRIBE 1
#define UNSUBSCRIBE 2
#define KILL 3

ServerData server = ServerData();
pthread_mutex_t locker;

void usage() {
  cout << "usage: ./server <server port>" << endl;
  cout << "example: ./server 51511" << endl;
  exit(EXIT_FAILURE);
}

void validateArgs(int argc, char **argv) {
  if (argc < 2) {
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

bool validateString(const string& s) {
  for (const char c : s) {
    string str(1, c);
    if (!isalnum(c) && !isspace(c) && str.find_first_not_of(",.?!:;+-*/=@#$%()[]{}") != string::npos) {
      return false;
    }
  }
  return true;
}

void sendMessage(ClientData  *cliData, string message) {
  size_t bufferLength = send(cliData->clientData->clientSocket, &message[0], strlen(&message[0]) + 1, 0);
  if (bufferLength != strlen(&message[0]) + 1) {
    perror("Error while sending message from server");
    exit(EXIT_FAILURE);
  }
}

int getMessageType(string message) {
  if (!validateString(message)) {
    return INVALID;
  } else if (message[0] == '+' && message.length() > 1) {
    return SUBSCRIBE;
  } else if (message[0] == '-' && message.length() > 1) {
    return UNSUBSCRIBE;
  } else if (message == "##kill") {
    return KILL;
  }
  return NORMAL;
}

bool subscribeTag(ClientData *cliData, string tag) {
  for (int i=0; i < cliData->subscriptions.size(); i++) {
    if ((cliData->subscriptions[i]) == tag) {
      sendMessage(cliData, "already subscribed +" + tag);
      return false;
    }
  }

  (cliData->subscriptions).push_back(tag);

  sendMessage(cliData, "subscribed +" + tag);
  return true;
}

bool unsubscribeTag(ClientData *cliData, string tag) {
  bool foundTag = false;
  int i;
  for (i=0; i < cliData->subscriptions.size(); i++) {
    if ((cliData->subscriptions[i]) == tag) {
      foundTag = true;
      break;
    }
  }
  if (!foundTag) {
    sendMessage(cliData, "not subscribed -" + tag);
    return false;
  }
  cliData->subscriptions.erase(cliData->subscriptions.begin() + i);
  sendMessage(cliData, "unsubscribed -" + tag);
  return true;
}

vector<string> getTagsFromMsg(string str) {
  string sub = "#";
  vector<int> positions; // holds all the positions that sub occurs within str
  vector<tuple<int, int>> beginAndEnd;
  
  // get starts
  bool safeDelete = false;
  size_t pos = str.find(sub, 0);
  
  while(pos != string::npos) {
    int posAsInt = static_cast<int>(pos);
    if (pos == str.length() - 1 || str[pos+1] == ' ') {
      pos = str.find(sub,pos+1);
      continue;
    }
    
    if (!(posAsInt != 0 && str[posAsInt-1] != ' ')) {
      safeDelete = true;
      positions.push_back(posAsInt);
    } else {
      if (safeDelete) {
        positions.pop_back();
        safeDelete = false;
      }
    }
    pos = str.find(sub,pos+1);
  }
  
  // get ends
  vector<int> ends;
  for (int i=0; i<positions.size(); i++) {
    for (int j=positions[i]; j<str.length(); j++) {
      if (str[j] == ' ') {
        ends.push_back(j-1);
        break;
      } else if (j == str.length() - 1) {
        ends.push_back(j);
        break;
      } else if (str[j] == '#' && j != positions[i]) {
        break;
      }
    }
  }
  
  // getTags
  vector<string> tags;
  for (int i=0; i<positions.size(); i++) {
      string tag = str.substr(positions[i]+1, (ends[i]-1) - positions[i]+1);
      tags.push_back(tag);
  }
  return tags;
}

bool isSubscribed(ClientData *cliData, vector<string> tags) {
  bool isSub = false;
  for (auto it = tags.begin(); it != tags.end(); it++) {
    for (int i=0; i<cliData->subscriptions.size(); i++) {
      if ((cliData->subscriptions[i]) == (*it)) {
        return true;
      }
    }
  }
  return false;
}

void sendMessageToSubscribers(vector<string> tags, string msg, ClientData *cliData) {
  for (auto it = server.clients.begin(); it != server.clients.end(); it++) {    
    if (isSubscribed((*it), tags)) {
      sendMessage((*it), msg);
    }
  }
}

void processMessage(ClientData *cliData, string msg) {  
  int msgType = getMessageType(msg);

  if (msgType == SUBSCRIBE) {
    msg.erase(0,1);
    subscribeTag(cliData, msg);
  
  } else if (msgType == UNSUBSCRIBE) {
    msg.erase(0,1);
    unsubscribeTag(cliData, msg);
  
  } else if (msgType == KILL) {
    exit(EXIT_SUCCESS);
  
  } else if (msgType == INVALID) {
    sendMessage(cliData, "Invalid Message!");

  } else {
    vector<string> tags = getTagsFromMsg(msg);
    sendMessageToSubscribers(tags, msg, cliData);
  }

}

void removeClient(ClientData *cliData) {
  pthread_mutex_lock(&locker); 
  int index = 0;
  for (index = 0; index < server.clients.size(); index++) {
    if (server.clients[index]->clientData->clientSocket == cliData->clientData->clientSocket) {
      break;
    }
  }

  server.clients.erase(server.clients.begin() + index);
  pthread_mutex_unlock(&locker);
}

void* clientThread(void *data) {
  ClientData *cliData = (ClientData *)data;
  char rcvMsgBuffer[BUFSZ];

  while (1) {
    memset(rcvMsgBuffer, 0, BUFSZ);
    size_t bufferLength = recv(cliData->clientData->clientSocket, rcvMsgBuffer, BUFSZ - 1, 0);
    
    if (bufferLength == 0) {
      cout << "cliente desconectado" << endl;
      removeClient(cliData);
      break;
    }

    printf("[msg], %d bytes: %s\n", (int)bufferLength, rcvMsgBuffer);
    processMessage(cliData, rcvMsgBuffer);

  }
  
  close(cliData->clientData->clientSocket);
  pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {

  validateArgs(argc, argv);
  pthread_mutex_init(&locker, NULL);
  SocketData serverData = createServer("v4", argv[1]);

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
    SocketData *sockData = new SocketData(cliSocket, clientStorage, &addrData);

    ClientData *cliData = new ClientData(sockData);

    pthread_mutex_lock(&locker);
    server.clients.push_back(cliData);
    pthread_mutex_unlock(&locker);

    pthread_t tid;
    pthread_create(&tid, NULL, clientThread, cliData);
  
  }


  return 0;
}
