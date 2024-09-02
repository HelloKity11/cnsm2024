#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/poll.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#define MAX_SERVER_SOCK 10
    
void errexit(char *format,...)
{ va_list args;
  va_start(args,format);
  vfprintf(stderr,format,args);
  va_end(args);
  exit(1);
}

/*
 adrinf のアドレスとポートの表示
 */
void print_addrinfo(struct addrinfo *adrinf)
{
    char hbuf[NI_MAXHOST];  /* ホスト */
    char sbuf[NI_MAXSERV];  /* ポート */
    int rc;

    /* ホストとポート番号を得る */
    if ( (rc = getnameinfo(adrinf->ai_addr, adrinf->ai_addrlen,
                        hbuf, sizeof(hbuf),
                        sbuf, sizeof(sbuf),
                    NI_NUMERICHOST | NI_NUMERICSERV)) !=0) {
        fprintf(stderr, "getnameinfo(): %s\n", gai_strerror(rc));
        return;
    }
    printf("[%s]:%s\n", hbuf, sbuf);
}

void print_sockaddr(struct sockaddr *sockadr,socklen_t len )
{
    char hbuf[NI_MAXHOST];  /* ホスト */
    char sbuf[NI_MAXSERV];  /* ポート */
    int rc;

    /* ホストとポート番号を得る */
    if ( (rc = getnameinfo(sockadr, len,
                        hbuf, sizeof(hbuf),
                        sbuf, sizeof(sbuf),
                    NI_NUMERICHOST | NI_NUMERICSERV)) !=0) {
        fprintf(stderr, "getnameinfo(): %s\n", gai_strerror(rc));
        return;
    }
    printf("[%s]:%s\n", hbuf, sbuf);
}

int gconnectTCP(char *host, char* port)
{
    int sock,err;
    struct addrinfo hints, *res0, *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;  
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    /* getaddrinfo で、AAAAおよびAレコードを取得*/
    if((err = getaddrinfo(host, port, &hints, &res0))!=0) {
    fprintf(stderr, "error : %s", gai_strerror(err));
    if(res0) freeaddrinfo(res0);
    return -1; /* retuning, not exit now */
    }

    /* getaddrinfoの結果を利用し、接続が成功するまで試行する */
    for (res = res0; res; res = res->ai_next) {  
       sock = socket (res->ai_family, res->ai_socktype, res->ai_protocol);  
       if (sock < 0) continue;  
       if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {  
               close (sock);  
               sock = -1;
               continue;
       }
       break; /* 1つ繋がればおしまい */
    }
    if(res0) freeaddrinfo(res0);
    return sock;
}
int gconnectTCPo(char *host, char* port,int nsec)
{
    int sock,err;
    struct addrinfo hints, *res0, *res;
    int connect_timeo(int , const struct sockaddr *, socklen_t , int );

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;  
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    /* getaddrinfo で、AAAAおよびAレコードを取得*/
    if((err = getaddrinfo(host, port, &hints, &res0))!=0) {
    fprintf(stderr, "error : %s", gai_strerror(err));
    if(res0) freeaddrinfo(res0);
    return -1; /* retuning, not exit now */
    }

    /* getaddrinfoの結果を利用し、接続が成功するまで試行する */
    for (res = res0; res; res = res->ai_next) {  
          sock = socket (res->ai_family, res->ai_socktype, res->ai_protocol);  
          if (sock < 0) continue;  
          if (connect_timeo(sock, res->ai_addr, res->ai_addrlen,nsec) < 0) {  
     // close (sock);  if fail, it close sock.
             sock = -1;
             continue;
          }
          break; /* 1つ繋がればおしまい */
    }
    if(res0) freeaddrinfo(res0);
    return sock;
}

/**
 adrinfなソケットを作ってbindしてlistenしてソケットを返す．失敗は-1
 */

static int make_server_socket(struct addrinfo *adrinf)
{
    int sock = -1;
    int on=1;
    int rc;
    char *nmfunc = "";

    /* socket作成 */
    sock = socket(adrinf->ai_family, 
        adrinf->ai_socktype, adrinf->ai_protocol);
    if (sock < 0) {
        nmfunc = "socket()";
        goto err_exit_;
    }
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR, &on,sizeof on);
    if(adrinf->ai_family == AF_INET6)
      setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof on);

    /* bind */
    rc = bind(sock, adrinf->ai_addr, adrinf->ai_addrlen);
    if (rc < 0) {
        if(errno == EADDRINUSE) return -1;
        nmfunc = "bind()";
        goto err_exit_;
    }

    /* listen */
    rc = listen(sock, 5);
    if (rc < 0) {
        nmfunc = "listen()";
        goto err_exit_;
    }

    return sock;

err_exit_:
    perror(nmfunc);
    if (0 <= sock) {
        close(sock);
    }
    return -1;
}


int gpassiveTCP(char *port, fd_set *fds)
{
    struct addrinfo hints;
    struct addrinfo *res = NULL;    
    struct addrinfo *adrinf;

    int server_socks[MAX_SERVER_SOCK];  
    int num_server_socks;       
    int fdsmax;
    int rc;
    int i;
    fd_set orgfds;
    extern int daemonmode;

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
        hints.ai_family = AF_UNSPEC;
        hints.ai_protocol = IPPROTO_TCP;

    if( (rc = getaddrinfo(NULL, port, &hints, &res)) !=0) {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(rc));
        return -1;
    }

/* 得られたアドレス全部にソケットを作る */
    for (adrinf = res, num_server_socks = 0;
            adrinf != NULL && num_server_socks < MAX_SERVER_SOCK;
                adrinf = adrinf->ai_next) {
        int sock;

        /* ソケット作成 */
        sock = make_server_socket(adrinf);
        if (sock < 0) {
            continue;
        }

        /* 情報プリント */
        if(!daemonmode){
        printf("socket#=%d\n",sock);
        print_addrinfo(adrinf); }

        server_socks[num_server_socks] = sock;
        ++num_server_socks;
    }

    if (res != NULL) freeaddrinfo(res);

    /* ソケットが結局１つも作れなかった */
    if (num_server_socks == 0) {
        fprintf(stderr, "no socket\n");
        return -1;
    }

    /* select() でaccept待ち */
    FD_ZERO(&orgfds);

    for (fdsmax = -1, i = 0; i < num_server_socks; ++i) {
        FD_SET(server_socks[i], &orgfds);
        if (fdsmax < server_socks[i])
            fdsmax = server_socks[i];
    }
    *fds = orgfds;  
    /* memcpy(fds, &orgfds, sizeof(fd_set)); */
    return fdsmax; /* >=0 */
    
}

/* poll version */
int gppassiveTCP(char *port, struct pollfd *pfd, int plen)
{
    struct addrinfo hints;
    struct addrinfo *res = NULL;    
    struct addrinfo *adrinf;

    int num_server_socks;       
    extern int daemonmode;
    int rc;

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
        hints.ai_family = AF_UNSPEC;
        hints.ai_protocol = IPPROTO_TCP;

    if( (rc = getaddrinfo(NULL, port, &hints, &res)) !=0) {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(rc));
        return -1;
    }

/* 得られたアドレス全部にソケットを作る */
    for (adrinf = res, num_server_socks = 0;
            adrinf != NULL && num_server_socks < MAX_SERVER_SOCK;
                adrinf = adrinf->ai_next) {
        int sock;

        /* ソケット作成 */
        sock = make_server_socket(adrinf);
        if (sock < 0) {
            continue;
        }

        /* 情報プリント */
        if(!daemonmode){
        printf("socket#=%d\n",sock);
        print_addrinfo(adrinf); }

                if(num_server_socks<plen) {
        pfd[num_server_socks].fd = sock;
        ++num_server_socks; }
    }

    if (res != NULL) freeaddrinfo(res);

    /* ソケットが結局１つも作れなかった */
    if (num_server_socks == 0) {
        fprintf(stderr, "no socket\n");
        return -1;
    }

    return num_server_socks; /* >0 */
    
}

int accept_one(fd_set *fds, int ms) {
  struct sockaddr_storage fsin;
  fd_set work;
  int i, ss, alen;

  while(1) {
    work = *fds;
    if(select(ms + 1, &work, NULL, NULL, NULL) < 0) {continue;}
    for(i = 0; i <= ms; i++) if(FD_ISSET(i, &work)) {
    alen = sizeof(fsin);
    if ((ss = accept(i, (struct sockaddr *)&fsin, (socklen_t*)&alen)) < 0)
      continue;
    return ss;
      } //for
  } //while
}

/* lib.c と同じ関数 */
static int asize = 0;
static char *aptr;
#define WBS 8
int writecmd(int fd,char *buf,int nbytes)
{ int nleft, nwritten; 
  char p[WBS], *ptr = p; /* big enough? */

  if(nbytes > WBS-2) {
    if (asize-2 > nbytes) ptr=aptr;
    else {
      if (asize) free(aptr); 
      asize=nbytes+2;
      asize = ((asize+3)>>2)<<2; /* 4byte bound */
      aptr=ptr=(char*)malloc(asize);
      if(ptr==0) errexit("malloc fail");
    } 
  }
  bcopy(buf,ptr,nbytes);ptr[nbytes] = '\r';ptr[nbytes+1] = '\n';
  nleft = nbytes + 2;
  while(nleft > 0) {
    nwritten=write(fd,ptr,nleft);
    if(nwritten <= 0) return nwritten;
    nleft -= nwritten; ptr += nwritten;
  }
  return (nbytes - nleft);
}
/*
int readcmd(int fd,char *ptr,int maxlen)
{ int n, rc; unsigned char c;
  for(n=1; n<maxlen; n++) {
    if((rc=read(fd,&c,1))==1) {*ptr++ = c;
    if(n>1 && c=='\n' && *(ptr-2)=='\r')break;}
    else if(rc==0) {if (n==1) return (0); else continue;}
    else return (-1);
  }
  *(ptr-2)=0; return(n-2); // kill \r\n and null terminate 
}
*/

#ifdef USEOLD
int readcmd(int fd, char *ptr, int maxlen)
{   int n, rc;
    unsigned char c;
    for (n = 0; n < maxlen; n++) {
    if ((rc = read(fd, &c, 1)) == 1) {
        if (n > 0 && c == '\n' && *(ptr - 1) == '\r') break;
        else *ptr++ = c;
    } else if (rc == 0) {
        if (n == 0) return (0);
        else continue; // is it ok?
    } else return (-1); //read error 
    }
    *(ptr - 1) = '\0';
    if (n == maxlen) {
      if (*(ptr - 1) == '\r') {
    if ((rc = read(fd, &c, 1)) == 1) {
      if (c == '\n') return n-1;
      else return n;
    } else return n-1; // 0 or -1 but OK
      } else return n;
    } else return n-1;
}
#else

#define min(a,b) (a>b)?b:a
#define BLEN 4096
int readcmd(int fd, char *ptr, int maxlen)
{   int n, rc, i, j; 
    unsigned char buf[BLEN]; //enough?
    char junk[BLEN]; //
    rc=0;
    while(1) {
        n=recv(fd,&buf[rc],sizeof(buf)-rc,MSG_PEEK); // only peek!
        //buf[n]='\0'; //for temp
        //printf("%d bytes read.[%s]\n",n,buf);
        if(n< 0) return -1; //error
        if(n==0) return  0; // is it ok??
//search \r\n
        for(i=(rc>0)?rc-1:0; i<n+rc-1; i++)  {
            if(buf[i]=='\r' && buf[i+1]=='\n') { //found
//read it to junk. we got buf already.
                j=read(fd,junk,i+2-rc); // skip read
//copy buf[0] to buf[i-1] but less than maxlen
                j=min(maxlen-1,i);
                strncpy(ptr,(char*)buf,j);
                *(ptr+j)='\0'; //null terminate
                //printf("%d bytes return.[%s]\n",j,ptr);
                return j; //length read, without \r\n
            }
        } //not found \r\n
        j=read(fd,junk,n); //read and skip it anyway
        rc+=n;
        if(rc>=BLEN) { //not found \r\n, too long
            return -1; //error ok? or 0?
        }
    }
}
#endif

void printpeer(int fd)
{
socklen_t len;
struct sockaddr_storage  addr;
struct sockaddr_in6 *s6;
struct sockaddr_in *s4;
char ipstr[INET6_ADDRSTRLEN];
int port;

len = sizeof addr;
if(getsockname(fd, (struct sockaddr*)&addr, &len)<0) 
{printf("getsockname error\n");return; }

if (addr.ss_family == AF_INET) {
    s4 = (struct sockaddr_in *)&addr;
    port = ntohs(s4->sin_port);
    inet_ntop(AF_INET, &(s4->sin_addr), ipstr, sizeof ipstr);
} else { 
    s6 = (struct sockaddr_in6 *)&addr;
    port = ntohs(s6->sin6_port);
    inet_ntop(AF_INET6, &(s6->sin6_addr), ipstr, sizeof ipstr);
}

printf("My   IP address: %s\n", ipstr);
printf("My   port      : %d\n", port);

if(getpeername(fd, (struct sockaddr*)&addr, &len)<0) 
{printf("getpeername error\n");return; }

if (addr.ss_family == AF_INET) {
    s4 = (struct sockaddr_in *)&addr;
    port = ntohs(s4->sin_port);
    inet_ntop(AF_INET, &(s4->sin_addr), ipstr, sizeof ipstr);
} else { 
    s6 = (struct sockaddr_in6 *)&addr;
    port = ntohs(s6->sin6_port);
    inet_ntop(AF_INET6, &(s6->sin6_addr), ipstr, sizeof ipstr);
}

printf("Peer IP address: %s\n", ipstr);
printf("Peer port      : %d\n", port);
}

/* for connect_timeo */
typedef void (*sig_t) (int);

static void connect_alarm(int);
static void err_msg(char *s) {fprintf(stderr,"%s\n",s);fflush(stderr);}

int
connect_timeo(int sfd, const struct sockaddr *sa, socklen_t salen, int nsec)
{
    sig_t   sig_save;
    int     n;

    sig_save = signal(SIGALRM, connect_alarm);
    if (alarm(nsec) != 0)
        err_msg("connect_timeo: alarm was already set");

    errno=0;
    if ( (n = connect(sfd, (struct sockaddr*)sa, salen)) < 0) {
        if (errno == EINTR)
            errno = ETIMEDOUT;
        close(sfd);
    }
    alarm(0);               /* turn off the alarm */
    signal(SIGALRM, sig_save);  /* restore previous signal handler */

    return(n);
}

static void
connect_alarm(int signo)
{
    (void)signo;
    return;     /* just interrupt the connect() */
}
