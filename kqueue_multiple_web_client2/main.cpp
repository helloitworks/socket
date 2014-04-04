#include "common.h"
#include "socket_kqueue.h"

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


int kq;

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
    sp_write(kq, fptr->f_fd, NULL, false);
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
    if ( (n = connect(fd, ai->ai_addr, ai->ai_addrlen)) < 0)
    {
        if (errno != EINPROGRESS)
            err_sys("nonblocking connect error");
        fptr->f_flags = F_CONNECTING;
        //register
        
        sp_add(kq, fptr->f_fd, NULL);
        sp_write(kq, fptr->f_fd, NULL, true);
        if (fd > maxfd)
            maxfd = fd;
        
    }
    else if (n >= 0)              /* connect is already done */
    {
        sp_add(kq, fptr->f_fd, NULL);
        sp_write(kq, fptr->f_fd, NULL, true);
        write_get_cmd(fptr);    /* write() the GET command */
    }
}


int find_file_index_by_event(struct kevent event)
{
    int i ;
    for (i = 0; i < nfiles; i++) {
        if (file[i].f_fd == event.ident)
        {
            return i;
        }
	}
    return 0;
    
}

int
main(int argc, char **argv)
{
	int		i, fd, n, maxnconn, flags, error, ret, index;
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
    
    kq = kqueue();
    
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
        
        struct kevent events[2];
        
        ret = kevent(kq, NULL, 0, events, nfiles, NULL);
        
		for (i = 0; i < ret; i++) {
            
			//fd = file[i].f_fd;
            struct kevent event = events[i];
            //kqueue返回的events list事件是不按照顺序的。所以要根据event找到file index。这一点跟select有很大的不同，因为select是按照顺序的
            index = find_file_index_by_event(event);
			flags = file[index].f_flags;
			if (flags == 0 || flags & F_DONE)
				continue;
            
            int fd = event.ident;
            int avail_bytes = event.data;
            
            if (flags & F_CONNECTING && event.filter == EVFILT_WRITE)
            {
				n = sizeof(error);
                //todo 这里不加socklen_t就编译不通过！
                ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&n);
				if ( ret < 0 || error != 0) {
					err_ret("nonblocking connect failed for %s",
							file[index].f_name);
				}
                /* 4connection established */
				printf("connection established for %s\n", file[index].f_name);
				
                write_get_cmd(&file[index]);/* write() the GET command */
                
                
            }
            else if (flags & F_READING && event.filter == EVFILT_READ)
            {
                memset(buf, 0, sizeof(buf));
				if ( (n = Read(fd, buf, sizeof(buf)-1)) == 0) {
					printf("end-of-file on %s\n", file[index].f_name);
					Close(fd);
					file[index].f_flags = F_DONE;	/* clears F_READING */
                    
                    struct kevent changes[1];
                    EV_SET(&changes[0], fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
                    ret = kevent(kq, changes, 1, NULL, 0, NULL);
                    
					nconn--;
					nlefttoread--;
				} else {
                    buf[n]= '\0';
					printf("read %d bytes from %s\n", n, file[index].f_name);
                    printf("buf = %s\n", buf);
				}
			}
            
		}
	}
	exit(0);
}

