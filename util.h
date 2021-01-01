#pragma once

#include <stdlib.h>
#include <arpa/inet.h>

bool addressParse(const char *address, const char *port, struct sockaddr_storage *storage);