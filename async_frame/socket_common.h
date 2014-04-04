//
//  socket_common.h
//  socket
//
//  Created by shenyixin on 14-4-3.
//  Copyright (c) 2014年 shenyixin. All rights reserved.
//

#ifndef socket_socket_common_h
#define socket_socket_common_h

#define MAX_INFO 128
// MAX_SOCKET will be 2^MAX_SOCKET_P
#define MAX_SOCKET_P 16
#define MAX_SOCKET (1<<MAX_SOCKET_P) //最多支持65536个socket

#define MAX_EVENT 64            //用于epoll_wait的第三个参数，即每次epoll返回的event个数
#define MIN_READ_BUFFER 64      //read最小分配的缓冲区大小

// 套接字状态
#define SOCKET_TYPE_INVALID 0
#define SOCKET_TYPE_RESERVE 1
#define SOCKET_TYPE_PLISTEN 2   //监听套接字，未加入epoll管理
#define SOCKET_TYPE_LISTEN 3    //监听套接字，已加入epoll管理
#define SOCKET_TYPE_CONNECTING 4   //尝试连接中的套接字
#define SOCKET_TYPE_CONNECTED 5    //已连接的套接，主动或被动(*connect, accept成功，
#define SOCKET_TYPE_HALFCLOSE 6
#define SOCKET_TYPE_PACCEPT 7
#define SOCKET_TYPE_BIND 8

// 套接字返回结果
#define SOCKET_CONNECTION 0
#define SOCKET_DATA 1
#define SOCKET_OPEN 2
#define SOCKET_ACCEPT 3
#define SOCKET_CLOSE 4
#define SOCKET_ERROR 5
#define SOCKET_EXIT 6

#define MALLOC malloc
#define FREE free

class socket_proxy;
typedef std::function<void (socket_proxy *s)> connection_callback_fp;
typedef std::function<void (socket_proxy *s)> accept_callback_fp;
typedef std::function<void (socket_proxy *s)> data_callback_fp;
typedef std::function<void (socket_proxy *s)> error_callback_fp;
typedef std::function<void (socket_proxy *s)> close_callback_fp;


static void socket_keepalive(int fd) {
	int keepalive = 1;
	setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive , sizeof(keepalive));
}


#endif
