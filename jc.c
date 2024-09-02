#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
int connectTCP(char*,char*);
void errexit(char*,...);
int readcmd(int,char*,int), writecmd(int,char*,int);

#define LINEBUF 1024
int  s; /* global FD */

void ugent(int signalType){
  char buf[LINEBUF];
  (void)signalType;
  recv(s, buf, sizeof(buf), MSG_OOB);
  if(buf[0] == 'M'){printf("osodashi\n"); exit(0); }
  printf("Error OOB\n"); exit(1);
}

void setsigurg()
{
  struct sigaction handler;         
  handler.sa_handler = ugent;
  sigemptyset(&handler.sa_mask); 
  sigaddset(&handler.sa_mask, SIGURG);
  handler.sa_flags = SA_RESTART;
  if(sigaction(SIGURG, &handler, 0) < 0) 
    errexit("sigaction:");
}

int main(int argc, char *argv[])
{
  int             waiting;
  char            hand;
  int             n;
  char            buf[LINEBUF];
  (void)	  argc;

  setsigurg();
  s = connectTCP(argv[1], argv[2]);
  fcntl(s,F_SETOWN, getpid());
  while (1) {
    waiting = 1;

    while (waiting) {
      n = readcmd(s, buf, sizeof(buf));
      /* if(n==0) continue; */ /* or exit? */
      if (n < 0) errexit("readcmd error: %s\n", strerror(errno));
      buf[n] = 0;
      switch (buf[0]) {
      case 'S':	waiting = 0;
        printf("Start of %s\n", &buf[2]); break;
      case '0':
        printf("End of Janken, you loser.\n"); exit(0); break;
      case 'W':
        printf("%s\n", buf); break;
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

again:
    fputs("input your hand: g or t or p\n", stdout);
    fgets(buf, LINEBUF, stdin);
    hand = toupper(buf[0]);
    if ((hand != 'G') && (hand != 'T') && (hand != 'P'))
      goto again;

    buf[0] = hand;
    buf[1] = '\0';
    writecmd(s, buf, strlen(buf));

gg:n = readcmd(s, buf, sizeof(buf));
    if (n == 0) goto gg;
    if (n < 0) errexit("kekkano readcmd error: %s\n", strerror(errno));
    buf[n] = 0;
    switch (buf[0]) {
    case 'X':
      printf("you lose %s\n", &buf[2]);
      /* exit(0); */ /* not exit here now */
      break;
    case '1':
      printf("you are the chanpion %s\n", &buf[2]);
      exit(0); break;
    case '0':
      printf("you finally lose %s\n", &buf[2]);
      exit(0); break;
    case 'A':
      printf("aiko de %s\n", &buf[2]);
      break;
    case 'O':
      printf("you win this time %s\n", &buf[2]);
      break;
    case 'N':
      printf("Janken Failure\n"); exit(0); break;
    default:
      if (n > 0) {
         printf("got n=%d,#%s#@%X@\n", n, buf, buf[0]);
         errexit("Protocol error:  %s\n", strerror(errno));
	} 
      break;
    } /* switch */
  } /* while 1 */
}
