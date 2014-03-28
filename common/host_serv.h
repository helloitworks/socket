//
//  host_serv.h
//  socket
//
//  Created by shenyixin on 14-3-26.
//  Copyright (c) 2014年 shenyixin. All rights reserved.
//

#ifndef socket_host_serv_h
#define socket_host_serv_h
#include "common.h"
struct addrinfo *
host_serv(const char *host, const char *serv, int family, int socktype);

#endif
