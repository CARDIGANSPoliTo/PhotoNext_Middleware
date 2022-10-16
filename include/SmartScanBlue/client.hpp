#pragma once
#ifndef CLIENT_H
#define CLIENT_H

/*******************************************************************************
* included libraries
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>

extern "C" {
#include "../../include/CommonLibraries/libutils/utils.h"
}
#include "listener.hpp"
#include <iostream>
#include <vector>
#include <list>
#include <thread>
#include <atomic>
#include <iterator>
#include <exception>
#include <chrono>
#include <cstdint>
extern string collectionName;
#endif
