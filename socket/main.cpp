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
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        err_sys("socket");
    }
    
    //sockaddr_in
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
    
    struct hostent *h;
    if ((h=gethostbyname(HOST)) == NULL) {
        err_sys("gethostbyname");
    }
    servaddr.sin_addr = *(struct in_addr *)h->h_addr;
    
    /*
     if(inet_pton(AF_INET, IPSTR, &servaddr.sin_addr) <= 0) {
        err_sys("inet_pton");
     }
     */
    
    //connect
	if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        err_sys("connet");
    }
    
    //send
    build_http_request(HOST, httpRequest, sizeof(httpRequest)/sizeof(int));
    if (send(sockfd,httpRequest,strlen(httpRequest), 0) < 0)
    {
        err_sys("send");
    }


	while(true)
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
	return 0;
}