#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif


static u_short portbase = 0; //not use base shift now

void errexit(char *format,...)
{ va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  exit(1);
}

static int connectsock(char *host,char *service,char *protocol, int doc,
		       struct sockaddr *cos)
{ struct hostent *phe;
  struct servent *pse;
  struct protoent *ppe;
  struct sockaddr_in sin;
  int s, type, sb = 1, tcp = 0;

  bzero((char*)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  if((pse = getservbyname(service, protocol)) != 0)
    sin.sin_port = pse->s_port;
  else if((sin.sin_port = htons((u_short)atoi(service))) == 0)
    errexit("can't get \"%s\" service entry\n", service);
  if((phe = gethostbyname(host)) != 0)
    bcopy(phe->h_addr, (char*)&sin.sin_addr, phe->h_length);
  else if((sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
    errexit("can't get \"%s\" host entry\n", host);
  if((ppe = getprotobyname(protocol)) == 0)
    errexit("can't get \"%s\" protocol entry\n", protocol);
  if(strcmp(protocol,"udp") == 0) type = SOCK_DGRAM;
  else if(strcmp(protocol,"udplite") == 0) type = SOCK_DGRAM;
  else {type = SOCK_STREAM; tcp = 1; }
  if((s = socket(PF_INET, type, ppe->p_proto)) < 0)
    errexit("can't create socket: %s\n", strerror(errno));
  if (tcp && setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &sb, sizeof(sb)) < 0)
    errexit("can't set socketoption: %s\n", strerror(errno));
  if(doc && connect(s, (struct sockaddr*)&sin, sizeof(sin)) < 0)
    errexit("can't connect to %s.%s: %s\n", host, service, strerror(errno));
  if(cos) bcopy(&sin, cos, sizeof(sin));
  return s;
}

static int passivesock(char *service, char *protocol, int qlen)
{ struct servent *pse;
  struct protoent *ppe;
  struct sockaddr_in sin;
  int s, type, on = 1, tcp = 0;

  bzero((char*)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  if((pse = getservbyname(service,protocol)) != 0)
    sin.sin_port = htons(ntohs((u_short)pse->s_port) + portbase);
  else if((sin.sin_port = htons((u_short)atoi(service))) == 0)
    errexit("can't get \"%s\" service entry\n", service);
  if((ppe = getprotobyname(protocol)) == 0)
    errexit("can't get \"%s\" protocol entry\n", protocol);
  if(strcmp(protocol,"udp") == 0) type = SOCK_DGRAM;
  else if(strcmp(protocol,"udplite") == 0) type = SOCK_DGRAM;
  else {type = SOCK_STREAM; tcp = 1;}
  if((s = socket(PF_INET, type, ppe->p_proto)) < 0)
    errexit("can't create socket: %s\n", strerror(errno));
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  if (tcp && setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on)) < 0)
    errexit("can't set socketoption: %s\n", strerror(errno));
  if(bind(s, (struct sockaddr*)&sin, sizeof(sin)) < 0)
    errexit("can't bind to %s port: %s\n", service, strerror(errno));
  if(tcp && listen(s, qlen) < 0)
    errexit("can't listen on %s port: %s\n", service, strerror(errno));
  return s;
}

int connectTCP(char *host, char *service)
{ return connectsock(host, service, "tcp", 1, 0); }
int connectUDP(char *host, char *service)
{ return connectsock(host, service, "udp", 1, 0); }
int connectUDPLITE(char *host, char *service)
{ return connectsock(host, service, "udplite", 1, 0); }
int noconnectUDP(char *host, char *service, struct sockaddr *cos)
{ return connectsock(host, service, "udp", 0, cos);}
int noconnectUDPLITE(char *host, char *service, struct sockaddr *cos)
{ return connectsock(host, service, "udplite", 0, cos);}
int passiveTCP(char *service, int qlen)
{return passivesock(service, "tcp", qlen);}
int passiveUDP(char *service)
{return passivesock(service, "udp", 0);}
int passiveUDPLITE(char *service)
{return passivesock(service, "udplite", 0);}
int activeTCP(char *service, int qlen)
{ static struct sockaddr_in fsin;
  static int s, ss;
  static socklen_t alen = sizeof(fsin);
  s = passivesock(service, "tcp", qlen);
  ss = accept(s, (struct sockaddr*)&fsin, &alen);
  return ss;
}

static int asize = 0;
static char *aptr;
#define WBS 8
int writecmd(int fd, char *buf, int nbytes)
{ int nleft, nwritten; 
  char p[WBS], *ptr = p; /* big enough? */

  if(nbytes > WBS - 2) {
    if (asize - 2 > nbytes) ptr = aptr;
    else {
      if (asize) free(aptr); 
      asize = nbytes + 2;
      asize = ((asize + 3) >> 2) << 2; /* 4byte bound */
      aptr = ptr = (char*)malloc(asize);
      if(ptr == 0) errexit("malloc fail");
    } 
  }
  bcopy(buf, ptr, nbytes); ptr[nbytes] = '\r';ptr[nbytes+1] = '\n';
  nleft = nbytes + 2;
  while(nleft > 0) {
    nwritten = write(fd, ptr, nleft);
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
  *(ptr-2)=0; return(n-2);
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
	    if (n == 0)	return (0);
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
if(getsockname(fd, (struct sockaddr*)&addr, &len) < 0) 
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

if(getpeername(fd, (struct sockaddr*)&addr, &len) < 0) 
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
