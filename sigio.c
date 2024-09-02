#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define BUFSIZE 4096

int sigflag = 0;

void sigio_func(int signalType)
{
  (void) signalType;
//  fprintf(stderr,"hadler called.type(%d)\n",signalType);fflush(stderr);
  sigflag = 1;
}

void  sys_err(char *s, char *t)
  {fprintf(stderr, "%s:%s\n", s, t); exit(1);}

/* ファイルディスクリプタ０から読んで１に書き出すプログラム */
int  main(int argc, char *argv[])
  { int n; (void)argc; (void)argv;
    char buffer[BUFSIZE];
    void sigio_func(int signalType);
    sigset_t set;
    struct sigaction handler;
    handler.sa_handler =  sigio_func;
    handler.sa_flags = SA_RESTART;
    sigemptyset(&handler.sa_mask); 
    sigaddset(&handler.sa_mask, SIGIO);
    if(sigaction(SIGIO, &handler, 0) < 0) 
		sys_err("Sigaction error", strerror(errno));
    if(fcntl(0, F_SETOWN, getpid()) < 0) 
		sys_err("F_SETOWN error", strerror(errno));
    if(fcntl(0, F_SETFL, O_NONBLOCK|FASYNC) < 0) 
		sys_err("F_SETFL NONBLOCL|ASYNC error", strerror(errno));
    for(;;) {
      //while(sigflag==0) sigpause(0);
      sigemptyset(&set);
      while(sigflag == 0) sigsuspend(&set);
      if((n = read(0, buffer, sizeof(buffer))) > 0){
        if(write(1, buffer, n) != n)
	  sys_err("write error", strerror(errno)); }
      else if(n < 0) {
	if(errno == EAGAIN || errno == EINTR) continue;
	sys_err("read error", strerror(errno)); }
      else if(n == 0) return 0;
      sigflag = 0;
      }
  }

