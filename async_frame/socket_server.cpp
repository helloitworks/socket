//
//  socket_server.cpp
//  socket
//
//  Created by shenyixin on 14-4-3.
//  Copyright (c) 2014年 shenyixin. All rights reserved.
//

#include "socket_server.h"

socket_server::socket_server()
{
    poll_fd efd = sp_create();
    this->alloc_id = 0;
    this->event_fd = efd;
}


//从socket池中获取一个空的socket,并为其分配一个id(0~2147483647即2^31-1)
int socket_server::reserve_id()
{
    //todo
    int id = __sync_add_and_fetch(&(this->alloc_id), 1);
    return id;
}

void * socket_server::poll()
{
    socket_proxy *s = NULL;
	for (;;)
    {
		int type = this->_poll(&s);
		switch (type)
        {
            case SOCKET_EXIT:
                return NULL;
            case SOCKET_DATA:
                s->data_callback(s);
                //free(result.data);
                break;
            case SOCKET_CLOSE:
                s->close_callback(s);
                break;
            case SOCKET_CONNECTION:
            {
                s->connection_callback(s);
                break;
            }
            case SOCKET_OPEN:
            {
                break;
            }
            case SOCKET_ERROR:
                //todo
                //printf("error(%lu) [id=%d]\n",result.opaque,result.id);
                break;
            case SOCKET_ACCEPT:
                //todo
                //printf("accept(%lu) [id=%d %s] from [%d]\n",result.opaque, result.ud, result.data, result.id);
                break;
		}
	}
    return NULL;
}

int socket_server::_poll(socket_proxy **s)
{
	for (;;)
    {
		if (this->event_index == this->event_n)
        {
			this->event_n = sp_wait(this->event_fd, this->ev, MAX_EVENT);
			this->event_index = 0;
			if (this->event_n <= 0)
            {
				this->event_n = 0;
				return -1;
			}
		}
		struct event *e = &this->ev[this->event_index++];
		*s = (socket_proxy *)e->s;
		if (s == NULL)
        {
			// dispatch pipe message at beginning
			continue;
		}
		switch ((*s)->type)
        {
            case SOCKET_TYPE_CONNECTING:
                return (*s)->report_connect();
            case SOCKET_TYPE_LISTEN:
//                if (report_accept(ss, s, result)) {
//                    return SOCKET_ACCEPT;
//                }
                break;
            case SOCKET_TYPE_INVALID:
                fprintf(stderr, "socket-server: invalid socket\n");
                break;
            default:
                if (e->write)
                {
                    int type = (*s)->send_buffer();  // 可写事件到来，从应用层缓冲区取出数据发送
                    if (type == -1)
                        break;
                    return type;
                }
                if (e->read)
                {
                    int type = (*s)->forward_message();
                    if (type == -1)
                        break;
                    return type;
                }
                break;
		}
	}
}


//为新预留的socket初始化，如果add为true还会将该套接字加入epoll管理
int socket_server::new_fd(socket_proxy *s, bool add)
{
    int id = this->reserve_id();
    //todo
    s->id = id;
    s->type = SOCKET_TYPE_RESERVE;
    s->size = MIN_READ_BUFFER;
    s->wb_size = 0;
	this->slot[id % MAX_SOCKET] = s;
	assert(s->type == SOCKET_TYPE_RESERVE);
    
	if (add)
    {
		if (sp_add(this->event_fd, s->fd, s))
        {
			s->type = SOCKET_TYPE_INVALID;
			return NULL;
		}
	}
    
	return 1;
}

