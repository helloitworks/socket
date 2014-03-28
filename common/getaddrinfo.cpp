//
//  getaddrinfo.cpp
//  socket
//
//  Created by shenyixin on 14-3-24.
//  Copyright (c) 2014年 shenyixin. All rights reserved.
//

#include "getaddrinfo.h"


bool GetAddrInfo(const char * host, sockaddr *sockaddr)
{
    int error;
    bool result = false;
    struct addrinfo hints = {0}, *res, *res0;
    
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    
    int ret = getaddrinfo(host, NULL, &hints, &res0);
    if (ret != 0)
    {
        //ex: getaddrinfo: nodename nor servname provided, or not known
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
        result = false;
        return result;
    }
    else
    {
        for (res = res0; res; res = res->ai_next)
        {
            
            *sockaddr = *res->ai_addr;
            
            /*
             * Use getnameinfo() to convert res->ai_addr to a
             * printable string.
             *
             * NI_NUMERICHOST means to present the numeric address
             * without doing reverse DNS to get a domain name.
             */
            /*下面的代码只用于打印*/
            char hostPrintableString[NI_MAXHOST];
            error = getnameinfo(res->ai_addr, res->ai_addrlen,
                                hostPrintableString, sizeof(hostPrintableString), NULL, 0, NI_NUMERICHOST);
            
            if (error) {
                fprintf(stderr, "%s\n", gai_strerror(error));
            } else {
                //Print the numeric address.
                printf("%s\n", hostPrintableString);
            }
        }
        result = true;
        freeaddrinfo(res0);
    }
    return result;
}

void TestGetAddrInfo()
{
    const char *host = "www.baidu.com";
    sockaddr sockaddr;
    bool ret = GetAddrInfo(host, &sockaddr);
    if (!ret)
    {
        printf("resolveHost error");
    }
}

/*
int main(int argc, const char * argv[])
{
    sockaddr sockaddr;
    bool ret = getAddrInfo(HOST, &sockaddr);
    if (!ret)
    {
        printf("resolveHost error");
    }
    
    return 0;
}
*/