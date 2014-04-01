//
//  main.cpp
//  client
//
//  Created by shenyixin on 14-4-1.
//  Copyright (c) 2014年 shenyixin. All rights reserved.
//

//ubuntu10.04 32bit

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
int main()
{
    int fd,ret,tmp,sendlen;
    struct sockaddr_in addr;
    char *buf;
    int sendBufLen = 1024*2;
    socklen_t optlen = sizeof(int);
    
    buf = (char *)malloc(1024 * 3000);
    fd = socket(AF_INET, SOCK_STREAM,0 );
    
    setsockopt(fd,SOL_SOCKET, SO_SNDBUF,(const char*)&sendBufLen, sizeof(int));
    getsockopt(fd,SOL_SOCKET, SO_SNDBUF,(int *)&tmp, &optlen);
    printf("send_tmp=%d,optlen=%d\n",tmp,(int)optlen);    //设置发送缓冲区2048
    
    getsockopt(fd,SOL_SOCKET, SO_RCVBUF,(int *)&tmp, &optlen);
    printf("recv_tmp=%d,optlen=%d\n",tmp,(int)optlen);
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9999);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //填上自己的IP
    
    ret = connect (fd, (struct sockaddr *)&addr, sizeof(addr));
    printf("connect return %d\n",ret);
    getchar();
    if (ret >= 0)
        sendlen = send(fd,buf,1024*3000,0);
    printf("sendlen=%d\n",sendlen);       //此处返回3072
    getchar();
    
    return 0;
}

