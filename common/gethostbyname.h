//
//  gethostbyname.h
//  socket
//
//  Created by shenyixin on 14-3-24.
//  Copyright (c) 2014年 shenyixin. All rights reserved.
//

#ifndef __socket__gethostbyname__
#define __socket__gethostbyname__

#include "common.h"

bool GetHostByName(const char * host, struct in_addr *in_addr);
void TestGetHostByName();
#endif /* defined(__socket__gethostbyname__) */
