#include "common.h"


#define MAXFILES    20
#define SERV        "80"    /* port number or service name */

struct file {
    char  *f_name;            /* filename */
    char  *f_host;            /* hostname or IPv4/IPv6 address */
    int    f_fd;              /* descriptor */
    int    f_flags;           /* F_xxx below */
} file[MAXFILES];

#define F_CONNECTING    1   /* connect() in progress */
#define F_READING       2   /* connect() complete; now reading */
#define F_DONE          4   /* all done */

//需要加host
#define GET_CMD     "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: Close\r\n\r\n"

/* globals */
int     nconn, nfiles, nlefttoconn, nlefttoread, maxfd;
fd_set  rset, wset;

/* function prototypes */
void    home_page(const char *, const char *);
void    start_connect(struct file *);
void    write_get_cmd(struct file *);


void
home_page(const char *host, const char *fname)
{
	int		fd, n;
	char	line[MAXLINE];
    
	fd = Tcp_connect(host, SERV);	/* blocking connect() */
    
	n = snprintf(line, sizeof(line), GET_CMD, fname, host);
	Writen(fd, line, n);
    
	for ( ; ; ) {
        memset(line, 0, sizeof(char) * MAXLINE);
		if ( (n = Read(fd, line, MAXLINE-1)) == 0)
			break;		/* server closed connection */
        line[MAXLINE - 1] = '\0';
		printf("read %d bytes of home page\n", n);
        printf("%s\n", line);
		/* do whatever with data */
	}
	printf("end-of-file on home page\n");
	Close(fd);
}

void
write_get_cmd(struct file *fptr)
{
    int     n;
    char    line[MAXLINE];
    memset(line, 0, sizeof(line));
    n = snprintf(line, sizeof(line), GET_CMD, fptr->f_name,fptr->f_host);
    Writen(fptr->f_fd, line, n);
    printf("wrote %d bytes for %s\n", n, fptr->f_name);
    
    fptr->f_flags = F_READING;          /* clears F_CONNECTING */
    
    FD_SET(fptr->f_fd, &rset);          /* will read server's reply */
    if (fptr->f_fd > maxfd)
        maxfd = fptr->f_fd;
}


void
start_connect(struct file *fptr)
{
    int             fd, flags, n;
    struct addrinfo *ai;
    
    ai = Host_serv(fptr->f_host, SERV, 0, SOCK_STREAM);
    
    fd = Socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    fptr->f_fd = fd;
    printf("start_connect for %s, fd %d\n", fptr->f_name, fd);
    
    /* 4Set socket nonblocking */
    flags = Fcntl(fd, F_GETFL, 0);
    Fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    
    /* 4Initiate nonblocking connect to the server. */
    if ( (n = connect(fd, ai->ai_addr, ai->ai_addrlen)) < 0) {
        if (errno != EINPROGRESS)
            err_sys("nonblocking connect error");
        fptr->f_flags = F_CONNECTING;
        FD_SET(fd, &rset);          /* select for reading and writing */
        FD_SET(fd, &wset);
        if (fd > maxfd)
            maxfd = fd;
        
    } else if (n >= 0)              /* connect is already done */
        write_get_cmd(fptr);    /* write() the GET command */
}




int
main(int argc, char **argv)
{
	int		i, fd, n, maxnconn, flags, error, ret;
	char	buf[MAXLINE];
	fd_set	rs, ws;
    
	if (argc < 5)
		err_quit("usage: web <#conns> <hostname> <homepage> <file1> ...");
	maxnconn = atoi(argv[1]);
    
	nfiles = min(argc - 4, MAXFILES);
	for (i = 0; i < nfiles; i++) {
		file[i].f_name = argv[i + 4];
		file[i].f_host = argv[2];
		file[i].f_flags = 0;
	}
	printf("nfiles = %d\n", nfiles);
    
	home_page(argv[2], argv[3]);
    
	FD_ZERO(&rset);
	FD_ZERO(&wset);
	maxfd = -1;
	nlefttoread = nlefttoconn = nfiles;
	nconn = 0;
    /* end web1 */
    /* include web2 */
	while (nlefttoread > 0) {
		while (nconn < maxnconn && nlefttoconn > 0) {
            /* 4find a file to read */
			for (i = 0 ; i < nfiles; i++)
				if (file[i].f_flags == 0)
					break;
			if (i == nfiles)
				err_quit("nlefttoconn = %d but nothing found", nlefttoconn);
			start_connect(&file[i]);
			nconn++;
			nlefttoconn--;
		}
        
		rs = rset;
		ws = wset;
		n = Select(maxfd+1, &rs, &ws, NULL, NULL);
        
		for (i = 0; i < nfiles; i++) {
			flags = file[i].f_flags;
			if (flags == 0 || flags & F_DONE)
				continue;
			fd = file[i].f_fd;
			if (flags & F_CONNECTING &&
				(FD_ISSET(fd, &rs) || FD_ISSET(fd, &ws))) {
				n = sizeof(error);
                //todo 这里不加socklen_t就编译不通过！
                ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&n);
				if ( ret < 0 || error != 0) {
					err_ret("nonblocking connect failed for %s",
							file[i].f_name);
				}
                /* 4connection established */
				printf("connection established for %s\n", file[i].f_name);
				FD_CLR(fd, &wset);		/* no more writeability test */
				write_get_cmd(&file[i]);/* write() the GET command */
                
			} else if (flags & F_READING && FD_ISSET(fd, &rs)) {
                memset(buf, 0, sizeof(buf));
				if ( (n = Read(fd, buf, sizeof(buf)-1)) == 0) {
					printf("end-of-file on %s\n", file[i].f_name);
					Close(fd);
					file[i].f_flags = F_DONE;	/* clears F_READING */
					FD_CLR(fd, &rset);
					nconn--;
					nlefttoread--;
				} else {
					printf("read %d bytes from %s\n", n, file[i].f_name);
                    printf("buf = %s\n", buf);
				}
			}
		}
	}
	exit(0);
}
