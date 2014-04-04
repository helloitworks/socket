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
void	 Fclose(FILE *);
FILE	*Fdopen(int, const char *);
char	*Fgets(char *, int, FILE *);
FILE	*Fopen(const char *, const char *);
void	 Fputs(const char *, FILE *);

#endif
