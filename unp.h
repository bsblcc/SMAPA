/*
        Computer Network Work I
            time server (UDP/TCP)

                            CS: Freezind Chen
                            @Nankai University
*/

#pragma once


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <pthread.h>

#include <fstream>
#include <openssl/sha.h>
using namespace std;
