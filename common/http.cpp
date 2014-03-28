//
//  http.c
//  socket
//
//  Created by shenyixin on 14-3-24.
//  Copyright (c) 2014年 shenyixin. All rights reserved.
//

#include "http.h"
void BuildHttpRequest(const char *host, char *httpRequest, int n)
{
    bzero(httpRequest, n);
	//构建请求信息
	strcat(httpRequest, "GET / HTTP/1.1\r\n");
    char httpRequestHostLine[200];
    sprintf(httpRequestHostLine, "Host: %s\r\n", host);
    strcat(httpRequest, httpRequestHostLine);
    //如果没有加上这一行，Connection是keep alive，服务器send完数据后，不会马上close，一直到超时才close。
    //那样子客户端recv要很很久才返回0
    strcat(httpRequest, "Connection: Close\r\n");
	strcat(httpRequest, "\r\n");
    
    //printf("httpRequest: %s\n",httpRequest);
}