//
//  common.h
//  socket
//
//  Created by shenyixin on 14-3-24.
//  Copyright (c) 2014年 shenyixin. All rights reserved.
//

#ifndef socket_common_h
#define socket_common_h



#include <stdio.h> /* stderr, stdout */
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>  /* basic socket definitions */
#include <netdb.h>          /* hostent struct, gethostbyname() */
#include <arpa/inet.h>      /* inet_ntoa() to format IP address */
#include <netinet/in.h>     /* in_addr structure */
#include <sys/_select.h>
#include <unistd.h>         /* close */
#include <fcntl.h>
#include <sys/event.h>



#include "error.h"
#include "http.h"
#include "gethostbyname.h"
#include "getaddrinfo.h"
#include "wrapsock.h"
#include "host_serv.h"
#include "wrapunix.h"
#include <algorithm> /*min max*/
using namespace std;
//#include "wrapsock.h"

/* Miscellaneous constants */
#define MAXLINE     4096    /* max text line length */
#define BUFFSIZE    8192    /* buffer size for reads and writes */

/* Following could be derived from SOMAXCONN in <sys/socket.h>, but many
 kernels still #define it as 5, while actually supporting many more */
#define LISTENQ     1024    /* 2nd argument to listen() */

//int setnonblock(int fd)
//{
//    int flags;
//    
//    flags = fcntl(fd,F_GETFL);
//    flags |= O_NONBLOCK;
//    fcntl(fd,F_SETFL,flags);
//    return 0;
//}


#endif
