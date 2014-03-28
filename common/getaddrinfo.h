//
//  getaddrinfo.h
//  socket
//
//  Created by shenyixin on 14-3-24.
//  Copyright (c) 2014年 shenyixin. All rights reserved.
//

#ifndef __socket__getaddrinfo__
#define __socket__getaddrinfo__

#include "common.h"
bool GetAddrInfo(const char * host, sockaddr *sockaddr);
void TestGetAddrInfo();

#endif /* defined(__socket__getaddrinfo__) */
