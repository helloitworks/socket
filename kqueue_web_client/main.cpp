//
//  main.cpp
//  socket
//
//  Created by shenyixin on 14-3-24.
//  Copyright (c) 2014年 shenyixin. All rights reserved.
//
#include "common.h"
#define HOST  "www.soso.com"
#define SERV         "80"         /* port number or service name */
#define BUFSIZE 1024*4

const int MAX_EVENT_COUNT = 5000;
const int MAX_RECV_BUFF = 65535;

int main(int argc, char **argv) {
    
	int sockfd;
	//struct sockaddr_in servaddr;
	char httpRequest[BUFSIZE], buf[BUFSIZE];
    
    //addrinfo
    struct addrinfo *ai = host_serv(HOST, SERV, 0, SOCK_STREAM);
    
    //socket
	sockfd = Socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    
    //connect
	Connect(sockfd, ai->ai_addr, ai->ai_addrlen);
    
    //todo
    //struct timespec ts;
    bool hasSend = false;
    bool hasClosed = false;
    int kq = kqueue();
    
    //register
    int ret = 0;
    struct kevent changes[1];
    EV_SET(&changes[0], sockfd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
    ret = kevent(kq, changes, 1, NULL, 0, NULL);
    
    EV_SET(&changes[0], sockfd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    ret = kevent(kq, changes, 1, NULL, 0, NULL);
    
    
	while(true)
    {
        if (hasClosed) {
            return 0;
        }
        struct kevent events[1];
        
        ret = kevent(kq, NULL, 0, events, 1, NULL);
        if (ret < 0)
        {
            
        }
        else
        {
            for (int i = 0; i < ret; i++)
            {
                struct kevent event = events[i];
                
                int fd = events[i].ident;
                int avail_bytes = events[i].data;
                
                if (event.filter == EVFILT_WRITE)
                {
                    //send
                    BuildHttpRequest(HOST, httpRequest, sizeof(httpRequest)/sizeof(int));
                    if (send(sockfd,httpRequest,strlen(httpRequest), 0) < 0)
                    {
                        err_sys("send");
                    }
                    EV_SET(&changes[0], sockfd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
                    ret = kevent(kq, changes, 1, NULL, 0, NULL);
                    if (ret == -1)
                    {
                        perror("kevent()");
                    }
                }
                
                if (event.filter == EVFILT_READ)
                {
                    memset(buf, 0, BUFFSIZE);
                    int n = read(sockfd, buf, BUFFSIZE-1);
                    if (n == 0 || n == -1)
                    {
                        hasClosed = true;
                        //close(sockfd);
                        //kevent(): Bad file descriptor
                        //EV_SET(&changes[0], sockfd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
                    }
                    else
                    {
                        printf("%s",buf);
                    }
                }
            }
        }
        
    }
	return 0;
}


