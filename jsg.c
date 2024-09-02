#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
int gpassiveTCP(char*,fd_set*);
void errexit(char*,...), usage(char*);
int readcmd(int,char*,int), writecmd(int,char*,int);

#define QLEN 5
#define LINEBUF 4096
#define GO(a,b) do{strcpy(buf,a);strcat(buf,han);writecmd(b,buf,strlen(buf));}while(0)


int main(int argc, char *argv[])/* a.out port# ninzuu */
{
  struct sockaddr_storage fsin;
  int             i, ms, ss, fd, alen, cc, nn,nin, nin1, fin,doing;
  char            buf[LINEBUF];
  char            han[256];
  int             fds[256], sfds[256];
  char            kati, make, gu, ti, pa;
  fd_set	  fdset, wfdset;
  setprogname(argv[0]);
  if(argc!=3) {usage("Usage: %s port\n");exit(1);}
  nin1 = nin = atoi(argv[2]);
  nn = 0;
  ms = gpassiveTCP(argv[1], &fdset);
  while (nin > 0) {
    wfdset=fdset;
    if(select(ms+1,&wfdset,NULL,NULL,NULL)<0) {continue;}
    for(i=0;i<=ms;i++) if(FD_ISSET(i,&wfdset)) {
      alen = sizeof(fsin);
      if ((ss = accept(i, (struct sockaddr *) &fsin, (socklen_t*)&alen)) < 0)
        errexit("accept failed: %s\n", strerror(errno));
      sfds[nn] = fds[nn] = ss; nn++;
      if (--nin > 0)
        for (fd = 0; fd < nn; fd++) {
          sprintf(buf, "W ato %d nin", nin);
          writecmd(fds[fd], buf, strlen(buf)); 
        }
    }
  } /* while. now nin == 0 */
  han[nn]='\0';
  doing=1; /* means OX */
  fin = 0; /* not 01 */
  while(doing) {
  doing=1; /* means OX */
  gu = ti = pa = 0;
  for (fd = 0; fd < nn; fd++)
    if(fds[fd] >= 0){
      sprintf(buf, "S %d nin janken", nin1);
      writecmd(fds[fd], buf, strlen(buf));}
  for (fd = 0; fd < nn; fd++)
    if (fds[fd] >= 0) {
      cc = readcmd(fds[fd], buf, LINEBUF);
/* check hand may help */
      (void) cc; // bad manner
      han[fd] = buf[0];
    }
  for (fd = 0; fd < nn; fd++) {
    if (fds[fd] >= 0) {
      if (han[fd] == 'G') gu++;
      else if (han[fd] == 'T') ti++;
      else if (han[fd] == 'P') pa++;
    }
  }
  if (gu && ti && pa) doing=2; /* aiko */
  else if (gu && ti) {
    kati = 'G'; make = 'T';
    if (gu == 1) fin = 1;
  } else if (ti && pa) {
    kati = 'T'; make = 'P';
    if (ti == 1) fin = 1;
  } else if (pa && gu) {
    kati = 'P'; make = 'G';
    if (pa == 1) fin = 1;
  } else doing = 2; /* aiko, all hand is same */

  (void) make; //bad manner
  switch (doing) {
  case 1:
  if (!fin) {
    nin=0;
    for (fd = 0; fd < nn; fd++)
      if (fds[fd] >= 0){
        if (han[fd] == kati) { 
          GO("O ",fds[fd]); 
          nin++;
        } else { 
          GO("X ",fds[fd]);
        }
      }
    for (fd = 0; fd < nn; fd++)
      if (fds[fd] >= 0  && han[fd] != kati){
        fds[fd]= -1; /* reset flags */ han[fd] = '-'; }
    nin1 = nin;
  }
  else {
  for (fd = 0; fd < nn; fd++)
    if (fds[fd] >= 0 && han[fd]==kati){GO("1 ", fds[fd]); }
    else {GO("0 ",sfds[fd]); }
  doing=0;
  }
  break;
case 2: 
  strcpy(buf,"A ");strcat(buf,han);
  for (fd = 0; fd < nn; fd++)
    if (fds[fd] >= 0){
      writecmd(fds[fd], buf, strlen(buf));
    }
  break;
  }
}
exit(0);
}
