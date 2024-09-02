#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
int gpassiveTCP(char*,fd_set*);
void errexit(char*,...);
int readcmd(int,char*,int), writecmd(int,char*,int);

#define QLEN 5
#define LINEBUF 4096
#define aaa(X) do{sprintf(work,"; (You.%d, Top.%d/%d%s)",  \
kk[X],top,total,th(total)); strcat(buf,work) ;\
buf[X+2]=tolower(buf[X+2]);} while(0)
#define bbb do{sprintf(work,"; (Y.%d, H.%d/%d)",  \
total[dd],wmax,totalm); strcat(buf,work);\
buf[dd+1]=tolower(buf[dd+1]);}while(0)
#define GO(a,b,c) do{strcpy(buf,a);strcat(buf,han);aaa(c);\
		     writecmd(b,buf,strlen(buf));}while(0)


char *th(int i) {
static char th[3];
strcpy(th,"th");
if((i%100)==11||(i%100)==12||(i%100)==13) {;}
else if((i%10)==1) strcpy(th,"st");
else if((i%10)==2) strcpy(th,"nd");
else if((i%10)==3) strcpy(th,"rd");
return th;
}

int main(int argc, char *argv[])/* a.out port# ninzuu */
{
  struct sockaddr_storage fsin;
  int             i, ms, ss, fd, alen, cc, nn,nin, nin1, fin,doing,ac;
  int		  nin2, kaisuu, kk[256], top,total;
  char            buf[LINEBUF];
  char            han[256],work[256];
  int             fds[256], sfds[256];
  char            kati, make, gu, ti, pa;
  int             oya,forkc=0,aiko=100;
  fd_set	  fdset, wfdset;
  int setsig=0;
  void setsigchild();
  if(argc<4) {fprintf(stderr,"Usage: %s port ninzu kaisu [aiko]\n",argv[0]);exit(1);}
  nin2 = nin1 = nin = atoi(argv[2]);
  kaisuu=atoi(argv[3]);
  if(argc==5) aiko=atoi(argv[4]);
  bzero(kk,sizeof(kk));
  nn = 0;
  ms = gpassiveTCP(argv[1], &fdset);
  oya=1;
  while(oya) {
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
    switch (fork()) {
    case 0:			/* child */
      //      close(ms); ERROR!!
      for(i=0;i<=ms;i++) if(FD_ISSET(i,&fdset)) close(i);
      FD_ZERO(&fdset); //needed?
      FD_ZERO(&wfdset); //needed?
      oya = 0;			/* break loop */
      break;
    default:			/* parent */
      forkc++;
      printf("%d%s game forked\n",forkc,th(forkc));
      for (fd = 0; fd < nn; fd++)
	close(fds[fd]);
      nin1 = nin = nin2;
      nn = 0;
      if(!setsig) {setsigchild();setsig=1;}
    }
  }

  top=total=0;
  han[nn]='\0';
  doing=1; /* means OX */
  fin = 0; /* not 01 */
  ac=0; /*aiko counter */
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
      (void) cc; //bad manner
      han[fd] = buf[0];
    }
  total++;
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

  (void) make; // bad manner
  switch (doing) {
  case 1:
  ac=0;
  if (!fin) {
    nin=0;
    for (fd = 0; fd < nn; fd++)
      if (fds[fd] >= 0){
        if (han[fd] == kati) { 
          GO("O ",fds[fd],fd); 
          nin++;
        } else { 
          GO("X ",fds[fd],fd);
        }
      }
    for (fd = 0; fd < nn; fd++)
      if ((fds[fd] >= 0)  && (han[fd] != kati)){
        fds[fd]= -1; /* reset flags */ han[fd] = '-'; }
    nin1 = nin;
  }
  else {
  /* 1nin kati */
    for (fd =0; fd < nn; fd++) 
      if (fds[fd] >= 0 && han[fd]==kati) break; /* fix fd to winner */
     kk[fd]++;
      if(kk[fd]>top) top=kk[fd];
      if(kk[fd]>=kaisuu) { /* FINAL WINNER */
        int k;
        for(k=0;k<nn;k++) if(k==fd) {GO("T ",fds[k],k);}
        else {GO("F ",sfds[k],k);}
      exit(0);
      }
  for (fd = 0; fd < nn; fd++)
    if (fds[fd] >= 0 && han[fd]==kati){GO("1 ", fds[fd],fd); }
    else {GO("0 ",sfds[fd],fd); }
  bcopy(sfds,fds,sizeof(fds)); nin1=nin2;fin=0;
  }
  break;
case 2: 
  if(++ac>=aiko) {strcpy(buf,"B ");strcat(buf,han);
    for (fd = 0; fd < nn; fd++)
      writecmd(sfds[fd], buf, strlen(buf));
    doing=0;} /* simply exit? */
  else {
  for (fd = 0; fd < nn; fd++)
    if (fds[fd] >= 0){
      strcpy(buf,"A ");strcat(buf,han);aaa(fd);
      writecmd(fds[fd], buf, strlen(buf));
    }
  }
  break;
  }
}
exit(0); /* never come? */
}
