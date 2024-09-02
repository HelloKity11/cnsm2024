#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>


void err_sys(char*);

#define	MAXLINE		4096	/* max text line length */
ssize_t                                         /* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n)
{
    size_t          nleft;
    ssize_t         nwritten;
    const char      *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;           /* and call write() again */
            else
                return(-1);                     /* error */
        }

        nleft -= nwritten;
        ptr   += nwritten;
    }
    return(n);
}


void
str_cli(FILE *fp, int sockfd)
{
    int	kq, i, n, nev, stdineof = 0, isfile;
    char	buf[MAXLINE];
    struct kevent	kev[2], tev[2];
    struct timespec	ts;
    struct stat		st;

    isfile = ((fstat(fileno(fp), &st) == 0) &&
              (st.st_mode & S_IFMT) == S_IFREG);

    EV_SET(&kev[0], fileno(fp), EVFILT_READ, EV_ADD, 0, 0, NULL);
    EV_SET(&kev[1], sockfd, EVFILT_READ, EV_ADD, 0, 0, NULL);

    kq = kqueue();
    ts.tv_sec = ts.tv_nsec = 0;
    kevent(kq, kev, 2, NULL, 0, &ts);

    for ( ; ; ) {
        nev = kevent(kq, NULL, 0, tev, 2, NULL);

        for (i = 0; i < nev; i++) {
            if (tev[i].ident == (uintptr_t) sockfd) {	/* socket is readable */
                if ( (n = read(sockfd, buf, MAXLINE)) == 0) {
                    if (stdineof == 1)
                        return;		/* normal termination */
                    else
                        err_sys("str_cli: server terminated prematurely");
                }

                write(fileno(stdout), buf, n);
            }

            if (tev[i].ident == (uintptr_t) fileno(fp)) {  /* input is readable */
                n = read(fileno(fp), buf, MAXLINE);
                if (n > 0)
                    writen(sockfd, buf, n);

                if (n == 0 || (isfile && n == tev[i].data)) {
                    stdineof = 1;
                    shutdown(sockfd, SHUT_WR);	/* send FIN */
                    tev[i].flags = EV_DELETE;
                    kevent(kq, &tev[i], 1, NULL, 0, &ts);	/* remove kevent */
                    continue;
                }
            }
        }
    }
}



int gconnectTCP(char*,char*);
void errexit(char*,...);
#define LINEBUF 1024

int main(int argc, char *argv[])
{   int s;
    (void)argc;
    setprogname(argv[0]);
    s = gconnectTCP(argv[1], argv[2]);
    str_cli(stdin,s);
    return 0;
}
