//
//  wrapunix.h
//  socket
//
//  Created by shenyixin on 14-3-31.
//  Copyright (c) 2014年 shenyixin. All rights reserved.
//

#ifndef socket_wrapunix_h
#define socket_wrapunix_h

int
Open(const char *pathname, int oflag, mode_t mode);

void
Close(int fd);

int
Fcntl(int fd, int cmd, int arg);


pid_t
Fork(void);

ssize_t
Read(int fd, void *ptr, size_t nbytes);


void
Write(int fd, void *ptr, size_t nbytes);


#endif
