//
//  tcp_client.h
//  socket
//
//  Created by shenyixin on 14-4-3.
//  Copyright (c) 2014年 shenyixin. All rights reserved.
//

#ifndef __socket__tcp_client__
#define __socket__tcp_client__

#include <iostream>
#include "socket_common.h"

struct write_buffer {
	struct write_buffer * next;
	char *ptr;  //指向当前块未发送缓冲区的首字节
	int sz;     //当前块未发送的字节数
	void *buffer;  //发送缓冲区
};

class socket_server;

class socket_proxy
{
public:
    int fd; //文件描述符
	int id; // 应用层维护一个与fd相对应的id
	int type; //socket类型（或状态）
	int size; // 下一次read操作要分配 的缓冲区大小
	int64_t wb_size;    //发送缓冲区中未发送的字节数
	uintptr_t opaque;   //在skynet中用于保存服务handle
	struct write_buffer * head; // 发送缓冲区链表头指针
	struct write_buffer * tail; // 发送缓冲区链表尾指针
   
    char *data; //接收到的数据
    const char *host;
    int port;
    socket_server *ss;
    
    connection_callback_fp connection_callback;
    close_callback_fp close_callback;
    data_callback_fp data_callback;
    
    socket_proxy(socket_server *, const char *host, int port);
    // 回调函数
    void set_connection_callback(const connection_callback_fp& cb) { connection_callback = cb; }
    void set_data_callback(const data_callback_fp& cb) { data_callback = cb; }
    void set_close_callback(const close_callback_fp& cb) { close_callback = cb; }

    int connect();
    int block_connect();
    void append_sendbuffer(char *buffer, int sz, int n);
    int report_connect();
    int forward_message();
    void force_close();
    int send_buffer();
    int send_socket(char *buffer, int sz);
    int64_t send(const void *buffer, int sz);
    
private:
    int _connect(bool blocking);

};

#endif /* defined(__socket__tcp_client__) */
