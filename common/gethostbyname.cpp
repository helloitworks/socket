//
//  gethostbyname.cpp
//  socket
//
//  Created by shenyixin on 14-3-24.
//  Copyright (c) 2014年 shenyixin. All rights reserved.
//

#include "gethostbyname.h"

bool GetHostByName(const char * host, in_addr *in_addr)
{
    bool result = false;
    int i = 0;
    
    struct hostent *h;     /* host information */
    
    if ((h = gethostbyname(host)) == NULL) {
        fprintf(stderr, "(mini) nslookup failed on '%s'\n", host);
        //exit(1);
        result = false;
        return result;
    }
    
    for (i = 0; h->h_addr_list[i] != 0; i++) {
        //只取第一个
        *in_addr = *(struct in_addr *)h->h_addr_list[0];
        
        //print only
        struct in_addr addr = *(struct in_addr *)h->h_addr_list[i];
        //addr.s_addr = *(unsigned long *)h->h_addr_list[i];
        printf("IP Address #%d: %s\n", i, inet_ntoa(addr));
    }
    
    result = true;
    return result;
}

void TestGetHostByName()
{
    const char *host = "www.baidu.com";
    in_addr in_addr;
    bool ret = GetHostByName(host, &in_addr);
    if (!ret)
    {
        printf("resolveHost error");
    }
    else
    {
        printf("IP Address: %s\n",inet_ntoa(in_addr));
    }
}

/*
int main(int argc, char **argv) {
    in_addr in_addr;
    bool ret = resolveHost(HOST, &in_addr);
    if (!ret)
    {
        printf("resolveHost error");
    }
    else
    {
        printf("IP Address: %s\n",inet_ntoa(in_addr));
    }
    //exit(0);
}
*/