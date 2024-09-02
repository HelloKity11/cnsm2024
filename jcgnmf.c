#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
int gconnectTCP(char*,char*);
void errexit(char*,...);
int readcmd(int,char*,int), writecmd(int,char*,int);


#define LINEBUF 1024
int  s; /* global FD */

void ugent(int signalType){
  char buf[LINEBUF + 1];
  (void)signalType;
  recv(s,buf,LINEBUF,MSG_OOB);
  if(buf[0]=='M'){printf("osodashi\n"); exit(0);}
  printf("Error OOB\n"); exit(1);
}

void setsigurg()
{
  struct sigaction handler;         
  handler.sa_handler = ugent;
  sigemptyset(&handler.sa_mask); 
  sigaddset(&handler.sa_mask,SIGURG);
  handler.sa_flags = SA_RESTART;
  if(sigaction(SIGURG, &handler,0)<0) errexit("sigaction:");
}

#define P printf("(PID=%d) ",getpid())

int main(int argc, char *argv[])
{
  int             waiting, rflag,num=1,k;
  char            hand;
  int             n;
  char            buf[LINEBUF + 1];
  if(argc!=4) {fprintf(stderr,"Usage: %s host port num\n",argv[0]);exit(1);}
  num=atoi(argv[3]);
  for(k=1;k<num;k++) if(fork()==0) break;
  srandom(getpid());
  setsigurg();
  s = gconnectTCP(argv[1], argv[2]);
  fcntl(s,F_SETOWN,getpid());
  while (1) {
    waiting = 1;

    while (waiting) {
      n = readcmd(s, buf, LINEBUF);
      /* if(n==0) continue; */ /* or exit? */
      if (n < 0) errexit("readcmd error: %s\n", strerror(errno));
      buf[n] = 0;
      switch (buf[0]) {
      case 'S':	waiting = 0;
        P;printf("Start of %s\n", &buf[2]); break;
      case '0':
        P;printf("End of this time Janken, you loser.%s\n",&buf[2]); break;
      case 'W':
        P;printf("%s\n", buf); break;
      case 'F':
        P;printf("FINAL loser.%s\n",&buf[2]); exit(0); break;
      case 'B':
        P;printf("too much aiko\n"); exit(0); break;
      case 'N':
        printf("Janken Failure\n"); exit(0); break;
      default:
        if (n > 0) {
           printf("got n=%d,=%s=^%X^\n", n, buf, buf[0]);
           errexit("Protocol error: %s\n", strerror(errno));
	  } 
        break;
      } /* switch */
    } /* while (waiting) */

/*
    int hflag=1;
    while(hflag) {
      fputs("input your hand: g or t or p\n", stdout);
      fgets(buf, LINEBUF+1, stdin);
      hand = toupper(buf[0]);
      if ((hand != 'G') && (hand != 'T') && (hand != 'P'))
        hflag=0;
    }
*/
    n = random()%3;
    if(n==0) hand='G'; else if(n==1) hand='T'; else hand='P';
    buf[0] = hand;
    buf[1] = '\0';
    writecmd(s, buf, strlen(buf));

    rflag=1;
    while(rflag) {
    n = readcmd(s, buf, LINEBUF);
    if (n == 0) continue;
    if (n < 0) errexit("kekkano readcmd error: %s\n", strerror(errno));
    else rflag = 0;
    }
    buf[n] = 0;
    switch (buf[0]) {
    case 'X':
      P;printf("you lose %s\n", &buf[2]);
      break;
    case '1':
      P;printf("you are the chanpion this time.%s\n", &buf[2]);
      break;
    case '0':
      P;printf("this janken, you lose %s\n", &buf[2]);
      break;
    case 'A':
      P;printf("aiko de %s\n", &buf[2]);
      break;
    case 'O':
      P;printf("you win this time %s\n", &buf[2]);
      break;
    case 'T':
      P;printf("YOU TOTAL CHANPION.%s\n",&buf[2]); exit(0); break;
    case 'F':
      P;printf("FINAL janken loser.%s\n",&buf[2]); exit(0); break;
    case 'N':
      printf("Janken Failure\n"); exit(0); break;
    case 'B':
      printf("too much aiko\n"); exit(0); break;
    default:
      if (n > 0) {
         printf("got n=%d,#%s#@%X@\n", n, buf, buf[0]);
         errexit("Protocol error:  %s\n", strerror(errno));
	} 
      break;
    } /* switch */
  } /* while 1 */
}
