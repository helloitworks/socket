//
//  main.cpp
//  daytime_server
//
//  Created by shenyixin on 14-4-1.
//  Copyright (c) 2014年 shenyixin. All rights reserved.
//

#include    "common.h"
#include    <time.h>

int
main(int argc, char **argv)
{
    int                 listenfd, connfd;
    struct sockaddr_in  servaddr;
    char                buff[MAXLINE];
    time_t              ticks;
    
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(9999); /* daytime server */
    
    Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
    
    Listen(listenfd, LISTENQ);
    
    for ( ; ; ) {
        connfd = Accept(listenfd, (SA *) NULL, NULL);
        
        ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        int i = 0;
        for(i = 0; i < strlen(buff); i++) {
            
            Write(connfd, buff+i, 1);
        }
        
        Close(connfd);
    }
}