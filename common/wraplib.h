//
//  wrapstdio.h
//  socket
//
//  Created by shenyixin on 14-4-3.
//  Copyright (c) 2014年 shenyixin. All rights reserved.
//

#ifndef socket_wrapstdio_h
#define socket_wrapstdio_h


/* prototypes for our stdio wrapper functions: see {Sec errors} */
const char      *Inet_ntop(int, const void *, char *, size_t);
void             Inet_pton(int, const char *, void *);

#endif
