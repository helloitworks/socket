//
//  socket_server.h
//  socket
//
//  Created by shenyixin on 14-4-3.
//  Copyright (c) 2014年 shenyixin. All rights reserved.
//

#ifndef __socket__socket_server__
#define __socket__socket_server__

#include "socket_poll.h"
#include "socket_server.h"
#include "socket_proxy.h"
#include "socket_common.h"
#include <assert.h>


//网际IP地址
union sockaddr_all {
	struct sockaddr s;
	struct sockaddr_in v4;
	struct sockaddr_in6 v6;
};

class socket_proxy;

class socket_server
{
public:
    int alloc_id;
    poll_fd event_fd;   //epoll fd
    int event_n;        //epoll_wait返回的事件个数
	int event_index;    //当前处理的事件序号，从0开始
	struct event ev[MAX_EVENT]; //用于epoll_wait
    socket_proxy* slot[MAX_SOCKET]; //应用层预先分配的socket数组（即socket池）
    char buffer[MAX_INFO];  //临时数据，比如保存新建连接的对待端的地址
    
    socket_server();
    void * poll();
    int reserve_id();
    int new_fd(socket_proxy *s, bool add);
    
private:
    int _poll(socket_proxy **s);

};
#endif /* defined(__socket__socket_server__) */
