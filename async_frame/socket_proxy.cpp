//
//  tcp_client.cpp
//  socket
//
//  Created by shenyixin on 14-4-3.
//  Copyright (c) 2014年 shenyixin. All rights reserved.
//

#include "socket_poll.h"
#include "socket_proxy.h"
#include "socket_server.h"
#include "socket_common.h"


socket_proxy::socket_proxy(socket_server *ss, const char *host, int port)
{
    this->ss = ss;
    this->host = host;
    this->port = port;
    this->head = NULL;
    this->tail = NULL;
}

int socket_proxy::connect()
{
    return _connect(false);
}

int socket_proxy::block_connect()
{
    return _connect(true);
}

int socket_proxy::_connect(bool blocking)
{
    bool ret;
    int status;
	struct addrinfo ai_hints;
	struct addrinfo *ai_list = NULL;
	struct addrinfo *ai_ptr = NULL;
	char port[16];
	sprintf(port, "%d", this->port);
	memset( &ai_hints, 0, sizeof( ai_hints ) );
	ai_hints.ai_family = AF_UNSPEC;
	ai_hints.ai_socktype = SOCK_STREAM;
	ai_hints.ai_protocol = IPPROTO_TCP;
    int sock= -1;

    // 获取地址列表，（不用gethostbyname、gethostbyadd,这两个函数仅支持IPV4)
    // 第一个参数是IP地址或主机名称，第二个参数是服务名（可以是端口或服务名称，如ftp、http等）
	status = getaddrinfo( this->host, port, &ai_hints, &ai_list );
	if ( status != 0 )
    {
		goto _failed;
	}
	for (ai_ptr = ai_list; ai_ptr != NULL; ai_ptr = ai_ptr->ai_next )
    {
		sock = socket( ai_ptr->ai_family, ai_ptr->ai_socktype, ai_ptr->ai_protocol );
		if ( sock < 0 )
        {
			continue;
		}
		socket_keepalive(sock);
		if (!blocking)
        {
			sp_nonblocking(sock);   // blocking为false,设置为非阻塞模式，即用非阻塞connect
		}
		status = ::connect( sock, ai_ptr->ai_addr, ai_ptr->ai_addrlen);
		if ( status != 0 && errno != EINPROGRESS)
        {
			close(sock);
			sock = -1;
			continue;   // 连接出错，跳过本循环，连接下一个地址
		}
		if (blocking)
        {
			sp_nonblocking(sock);   //到此为止，不管blocking是真是假，都设置为非阻塞模式，即IO模型使用的是non blocking
		}
		break;
	}
    
	if (sock < 0)
    {
		goto _failed;
	}
    
    this->fd = sock;

	ret = ss->new_fd(this, true);   // 加入epoll管理
    if (!ret)
    {
        close(sock);
        goto _failed;
    }
	if(status == 0)
    {   //说明connect已连接成功
		this->type = SOCKET_TYPE_CONNECTED;
		struct sockaddr * addr = ai_ptr->ai_addr;
		void * sin_addr = (ai_ptr->ai_family == AF_INET) ? (void*)&((struct sockaddr_in *)addr)->sin_addr : (void*)&((struct sockaddr_in6 *)addr)->sin6_addr;
		if (inet_ntop(ai_ptr->ai_family, sin_addr, ss->buffer, sizeof(ss->buffer)))
        {
			//result->data = ss->buffer;
		}
		freeaddrinfo( ai_list );
		return SOCKET_OPEN;
	}
    else
    {    // 说明非阻塞套接字尝试连接中
		this->type = SOCKET_TYPE_CONNECTING;
		sp_write(ss->event_fd, this->fd, this, true);   // 非阻塞字尝试连接中，必需将关注其可写事件，稍后epoll触发才可以捕捉到已连接
	}
    
	freeaddrinfo( ai_list );
	return -1;
_failed:
	freeaddrinfo( ai_list );
	//ss->slot[id % MAX_SOCKET].type = SOCKET_TYPE_INVALID;
    this->type = SOCKET_TYPE_INVALID;
	return 0;
}

//强制关闭套接字
void socket_proxy::force_close()
{
	if (this->type == SOCKET_TYPE_INVALID)
    {
		return;
	}
	assert(this->type != SOCKET_TYPE_RESERVE);
    //销毁发送缓冲区
	struct write_buffer *wb = this->head;
	while (wb)
    {
		struct write_buffer *tmp = wb;
		wb = wb->next;
		FREE(tmp->buffer);
		FREE(tmp);
	}
	this->head = this->tail = NULL;
	if (this->type != SOCKET_TYPE_PACCEPT && this->type != SOCKET_TYPE_PLISTEN)
    {
		sp_del(this->ss->event_fd, this->fd); //epoll取消关注该套接字
	}
    //SOCKET_TYPE_BIN类型不需要close(比如stdin,stdout等)，其它socket类型需要close
	if (this->type != SOCKET_TYPE_BIND)
    {
		close(this->fd);
	}
	this->type = SOCKET_TYPE_INVALID;
}

// 可读事件到来，执行该函数
// return -1 (ignore) when error
int socket_proxy::forward_message()
{
	int sz = this->size;
	char * buffer = (char *)MALLOC(sz);
	int n = (int)read(this->fd, buffer, sz);
	if (n<0)
    {
		FREE(buffer);
		switch(errno)
        {
            case EINTR:
                break;
            case EAGAIN:
                fprintf(stderr, "socket-server: EAGAIN capture.\n");
                break;
            default:
                // close when error
                //tood
                force_close(); //错误则强制关闭
                return SOCKET_ERROR;
		}
		return -1;
	}
	if (n==0)
    { // 对等端关闭
		FREE(buffer);
        //todo
		force_close();
		return SOCKET_CLOSE;
	}
    
	if (this->type == SOCKET_TYPE_HALFCLOSE)
    {
		// discard recv data
		FREE(buffer);
		return -1;
	}
    
	if (n == sz)
    {
		this->size *= 2;
	}
    else if (sz > MIN_READ_BUFFER && n*2 < sz)
    {
		this->size /= 2;   // s->size的最小值为MIN_READ_BUFFER
	}
    
    this->data = buffer;
	return SOCKET_DATA;
}

// 尝试连接中的套接字可写事件发生，可能是连接成功，也可能是连接出错
int socket_proxy::report_connect()
{
	int error;
	socklen_t len = sizeof(error);
	int code = getsockopt(this->fd, SOL_SOCKET, SO_ERROR, &error, &len);
	if (code < 0 || error)
    {
		force_close();
		return SOCKET_ERROR;
	}
    else
    {
		this->type = SOCKET_TYPE_CONNECTED;

		sp_write(this->ss->event_fd, this->fd, this, false);    // 连接成功，取消关注可写事件
		union sockaddr_all u;
		socklen_t slen = sizeof(u);
		if (getpeername(this->fd, &u.s, &slen) == 0)
        {
			void * sin_addr = (u.s.sa_family == AF_INET) ? (void*)&u.v4.sin_addr : (void *)&u.v6.sin6_addr;
			if (inet_ntop(u.s.sa_family, sin_addr, this->ss->buffer, sizeof(this->ss->buffer)))
            {
				this->data = this->ss->buffer;
				return SOCKET_CONNECTION;
			}
		}
		this->data = NULL;
		return SOCKET_CONNECTION;
	}
}



int64_t socket_proxy::send(const void *buffer, int sz)
{
    this->send_socket((char *)buffer, sz);
    return this->wb_size;
}

void socket_proxy::append_sendbuffer(char *buffer, int sz, int n)
{
	struct write_buffer * buf = (write_buffer *)malloc(sizeof(*buf));
	buf->ptr = buffer+n;
	buf->sz = sz - n;
	buf->buffer = buffer;
	buf->next = NULL;
	this->wb_size += buf->sz;
	if (this->head == NULL)
    {
		this->head = this->tail = buf;
	}
    else
    {
		assert(this->tail != NULL);
		assert(this->tail->next == NULL);
		this->tail->next = buf;
		this->tail = buf;
	}
}


//执行send事件
int socket_proxy::send_socket(char *buffer, int sz)
{
	if (this->type == SOCKET_TYPE_INVALID
		|| this->type == SOCKET_TYPE_HALFCLOSE
		|| this->type == SOCKET_TYPE_PACCEPT )
    {
		//FREE(buffer);
		return -1;
	}
	assert(this->type != SOCKET_TYPE_PLISTEN && this->type != SOCKET_TYPE_LISTEN);
    char * buf = (char *)malloc(sz);
    memcpy(buf, buffer, sz);
	if (this->head == NULL)
    {
		int n = write(this->fd, buf, sz);
        //printf("n=%d, errno = %d", n, errno);
		if (n<0)
        {
			switch(errno)
            {
                case EINTR:
                case EAGAIN:
                    n = 0;
                    break;
                default:
                    //fprintf(stderr, "socket-server: write to %d (fd=%d) error.",id,s->fd);
                    force_close();
                    return SOCKET_CLOSE;
			}
		}
		if (n == sz)
        {
			FREE(buf);
			return -1;
		}
		this->append_sendbuffer(buf, sz, n);
		sp_write(this->ss->event_fd, this->fd, this, true);
	}
    else
    {
        printf("append");
		this->append_sendbuffer(buf, sz, 0);
        
	}
	return -1;
}

// 可写事件到来，从应用层发送缓冲区取数据发送
int socket_proxy::send_buffer()
{
	while (this->head)
    {
		struct write_buffer * tmp = this->head;
		for (;;)
        {
			int sz = write(this->fd, tmp->ptr, tmp->sz);
			if (sz < 0)
            {
				switch(errno)
                {
                    case EINTR:
                        continue;
                    case EAGAIN:    // 内核发送缓冲区已满
                        return -1;
				}
				force_close();
				return SOCKET_CLOSE;
			}
			this->wb_size -= sz;   //发送缓冲区未发的字节数需要更新
			if (sz != tmp->sz)
            {    // 未将该块缓冲区完全发送出去
				tmp->ptr += sz; // 该块缓冲区未发送数据首地址更新
				tmp->sz -= sz;  // 当前块未发送字节数更新
				return -1;
			}
			break;
		}
		this->head = tmp->next;
		FREE(tmp->buffer);
		FREE(tmp);
	}
	this->tail = NULL;
	sp_write(this->ss->event_fd,this->fd, this, false);
    
	if (this->type == SOCKET_TYPE_HALFCLOSE)
    {
		force_close();
		return SOCKET_CLOSE;
	}
    
	return -1;
}

