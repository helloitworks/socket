//
//  main.cpp
//  async_frame
//
//  Created by shenyixin on 14-4-3.
//  Copyright (c) 2014年 shenyixin. All rights reserved.
//

#include <iostream>
#include "common.h"
#include "socket_server.h"
#include "socket_proxy.h"
#include "socket_common.h"

// 开一个线程poll
static void * _poll(void * ud)
{
	socket_server *ss = (socket_server *)ud;
    //ss->poll里面是无限循环，相当于infinite loop
    ss->poll();
    return NULL;
}

void on_connection(socket_proxy *s)
{
    printf("on_connection[id=%d] %s\n",s->id, s->data);
    const char *buffer = "it is great";
    
    s->send(buffer, strlen(buffer));
}


void on_connection2(socket_proxy *s)
{
    printf("on_connection[id=%d] %s\n",s->id, s->data);
    const char *buffer = "it is great";
    
    s->send(buffer, strlen(buffer));
}

void on_close(socket_proxy *s)
{
    printf("on_close[id=%d]\n",s->id);
}

void on_data(socket_proxy *s)
{
    
    printf("on_data[id=%d] %s\n",s->id,s->data);
}

int main(int argc, const char * argv[])
{
    socket_server *ss = new socket_server();
    
    pthread_t pid;
	pthread_create(&pid, NULL, _poll, ss);
    
    socket_proxy *s = new socket_proxy(ss, "127.0.0.1", 9877);
    s->set_connection_callback(std::bind(&on_connection, std::placeholders::_1));
    s->set_close_callback(std::bind(&on_close, std::placeholders::_1));
    s->set_data_callback(std::bind(&on_data, std::placeholders::_1));
    s->connect();
   	printf("connecting \n");
    
    socket_proxy *s2 = new socket_proxy(ss, "127.0.0.1", 9877);
    s2->set_connection_callback(std::bind(&on_connection2, std::placeholders::_1));
    s2->set_close_callback(std::bind(&on_close, std::placeholders::_1));
    s2->set_data_callback(std::bind(&on_data, std::placeholders::_1));
    s2->connect();
    printf("connecting \n");

	pthread_join(pid, NULL);

    return 0;
}

