#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
int passiveTCP(char*,int);
void errexit(char*,...);
int readcmd(int,char*,int), writecmd(int,char*,int);


#define QLEN 5
#define LINEBUF 4096
#define EBUF 256
#define GO(a,b) do{strcpy(buf,a);strcat(buf,han);\
                writecmd(b,buf,strlen(buf));}while(0)

void sendM(int fd)
{char mes = 'M';
  send(fd, &mes, 1, MSG_OOB);
}

int main(int argc, char *argv[])/* a.out port# ninzuu */
{
  struct sockaddr_in fsin;
  (void)	  argc;
  int             ms, ss, fd, alen, cc, nn, nin, nin1, fin;
  char            buf[LINEBUF];
  char            han[EBUF];
  int             fds[EBUF], sfds[EBUF];
  char            kati, make, gu, ti, pa;
  nin1 = nin = atoi(argv[2]);
  nn = 0;
  ms = passiveTCP(argv[1], QLEN);
  fin = 0;
  while (nin > 0) {
    alen = sizeof(fsin);
    if ((ss = accept(ms, (struct sockaddr *) &fsin, 
		(socklen_t*)&alen)) < 0)
      errexit("accept failed: %s\n", strerror(errno));
    sfds[nn] = fds[nn] = ss; nn++;
    if (--nin > 0)
      for (fd = 0; fd < nn; fd++) {
        sprintf(buf, "W ato %d nin", nin);
        writecmd(fds[fd], buf, strlen(buf)); 
      }
  } /* while. now nin == 0 */
  han[nn] = '\0';
 top:gu = ti = pa = 0;
  for (fd = 0; fd < nn; fd++)
    if(fds[fd] >= 0){
      sprintf(buf, "S %d nin janken", nin1);
      writecmd(fds[fd], buf, strlen(buf)); }
  for (fd = 0; fd < nn; fd++)
    if (fds[fd] >= 0) {
      cc = readcmd(fds[fd], buf, sizeof(buf));
      (void) cc; //bad manner, check cc
      han[fd] = buf[0];
    }
  for (fd = 0; fd < nn; fd++) {
    if (fds[fd] >= 0) {
      if (han[fd] == 'G') gu++;
      else if (han[fd] == 'T') ti++;
      else if (han[fd] == 'P') pa++;
      if (gu && ti && pa) goto aiko;
    }
  }
  if (gu && ti) {
    kati = 'G'; make = 'T';
    if (gu == 1) fin = 1;
  } else if (ti && pa) {
    kati = 'T'; make = 'P';
    if (ti == 1) fin = 1;
  } else if (pa && gu) {
    kati = 'P'; make = 'G';
    if (pa == 1) fin = 1;
  } else goto aiko; /* all same hand, so aiko */

  (void) make;
  if (!fin) {
    nin = 0;
    for (fd = 0; fd < nn; fd++)
      if (fds[fd] >= 0){
        if (han[fd] == kati) { 
          GO("O ", fds[fd]); 
          nin++;
        } else { 
          GO("X ", fds[fd]);
        }
      }
    for (fd = 0; fd < nn; fd++)
      if (fds[fd] >= 0  && han[fd] != kati) {
        fds[fd] = -1; /* reset flags */ han[fd] = '-'; }
    nin1 = nin;
    goto top; /* next janken */
  }
  /* final winner decided */
  for (fd = 0; fd < nn; fd++)
    if (fds[fd] >= 0 && han[fd] == kati) {GO("1 ", fds[fd]); }
    else {GO("0 ",sfds[fd]); }
  exit(0);

aiko:
  for (fd = 0; fd < nn; fd++)
    if (fds[fd] >= 0){
      GO("A ", fds[fd]);
    }
  goto top; /* next janken */
}

