//
//  http.c
//  socket
//
//  Created by shenyixin on 14-3-24.
//  Copyright (c) 2014年 shenyixin. All rights reserved.
//

#include "http.h"
void build_http_request(const char *host, char *http_request, int n)
{
    bzero(http_request, n);
	//构建请求信息
	strcat(http_request, "GET / HTTP/1.1\r\n");
    char host_line[200];
    sprintf(host_line, "Host: %s\r\n", host);
    strcat(http_request, host_line);
    //如果没有加上这一行，Connection是keep alive，服务器send完数据后，不会马上close，一直到超时才close。
    //那样子客户端recv要很很久才返回0
    strcat(http_request, "Connection: Close\r\n");
	strcat(http_request, "\r\n");
    
    //printf("http_request: %s\n",http_request);
}