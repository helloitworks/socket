//
//  main.cpp
//  socket
//
//  Created by shenyixin on 14-3-24.
//  Copyright (c) 2014年 shenyixin. All rights reserved.
//
#include "common.h"
#define HOST  "www.soso.com"
#define IPSTR "218.30.103.177"
#define PORT 80
#define BUFSIZE 1024*4

int main(int argc, char **argv) {
    
	int sockfd;
	struct sockaddr_in servaddr;
	char httpRequest[BUFSIZE], buf[BUFSIZE];
    
    //socket
	sockfd = Socket(AF_INET, SOCK_STREAM, 0);
    
    //sockaddr_in
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
    
    struct hostent *h;
    if ((h=gethostbyname(HOST)) == NULL) {
        err_sys("gethostbyname");
    }
    servaddr.sin_addr = *(struct in_addr *)h->h_addr;
    
    //connect
	Connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    
    struct timeval tv;
	fd_set rset;
    fd_set wset;
    bool hasSend = false;
    
	while(true)
    {
        //因为select返回后tv的值是未定义的，所以每次需要重设一下
        /*
         时间初始化为2秒，假设在1秒后发上了事件，则select将会返回并将tv的时间变成上次阻塞的剩余时间，即1秒，然后再进行监视套接字。这是因为linux系统对select()的实现中会修改参数tv为剩余时间。所以在循环内部使用函数select的时候一定要在循环内部初始化时间参数。
         */
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        
        FD_ZERO(&rset);
        FD_SET(sockfd, &rset);

        FD_ZERO(&wset);
        //请确保只有在你有数据发送的情况下才设置write_flag这个描述字集合，因为socket一量创建总是可写的。也就是说，如果你设置了这个参数，select将不会等待，而是马上返回并一直循环，它将抢占CPU99%的利用率，这是不允许的。
        if(!hasSend)FD_SET(sockfd, &wset);
        
        Select(sockfd + 1, &rset, &wset, NULL, &tv);
        if(FD_ISSET(sockfd, &rset))
        {
            memset(buf, 0, BUFSIZE);
            int ret = recv(sockfd, buf, BUFSIZE-1, 0);
            if (ret == -1)
            {
                perror("recv失败");
            }
            else if (ret == 0)
            {
                close(sockfd);
                printf("读取数据报文时发现远端关闭，该线程终止！\n");
                return 0;
            }
            else
            {
                printf("ret = %d\n", ret);
                printf("%s\n", buf);
            }
        }
        if(FD_ISSET(sockfd, &wset))
        {
            //send
            BuildHttpRequest(HOST, httpRequest, sizeof(httpRequest)/sizeof(int));
            if (send(sockfd,httpRequest,strlen(httpRequest), 0) < 0)
            {
                err_sys("send");
            }
            hasSend = true;
            FD_CLR(sockfd, &wset);
            printf("wset");
        }
    }
    
	return 0;
}