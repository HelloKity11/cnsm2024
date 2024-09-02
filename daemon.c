#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
//#include <varargs.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/param.h>
#include <errno.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#ifdef __sun
#include <sys/ttold.h>
//int open(const char *path, int oflag, /* mode_t mode */...);
#endif

void err_sys(char *s)
{(void)s;} /* daemonの場合は端末に書き出せない。
syslog等を利用するか、ファイルを自分で開いて書く*/

int daemonmode=0;

static void sig_child(int s)
{ int pid;
  int status;
  (void)s;
  while((pid=wait3(&status,WNOHANG,(struct rusage*)0)>0)) ;
}

void setsigchild(void)
{ struct sigaction act;
  act.sa_handler=sig_child;
  act.sa_flags=SA_NOCLDSTOP | SA_RESTART;
  sigaction(SIGCHLD, &act, NULL);
}

void daemon_start(int ignsigcld, int closeall)
{  register int childpid,fd;
#ifdef SIGTTOU
  signal(SIGTTOU,SIG_IGN);
#endif
#ifdef SIGTTIN
  signal(SIGTTIN,SIG_IGN);
#endif
#ifdef SIGTSTP
  signal(SIGTSTP,SIG_IGN);
#endif
  if((childpid=fork())<0)
    err_sys("can't fork first childre");
  else if (childpid>0) exit(0); 
#ifdef __FreeBSD__
 // if(setpgrp(0,getpid())== -1)
  if(setpgid(0,getpid())== -1)
#elif __NetBSD__
  if(setpgid(0,getpid())== -1)
#elif __OpenBSD__
  if(setpgid(0,getpid())== -1)
#else
  if(setpgrp()== -1)
#endif
    err_sys("can't change process gorup");
  if((fd=open("/dev/tty",O_RDWR))>=0) 
    {ioctl(fd,TIOCNOTTY,(char*)NULL);close(fd);}
  if(closeall)
    for(fd=0;fd<NOFILE;fd++) close(fd);
  errno=0;
/*  chdir("/"); */  /*必要なら行う*/
  umask(0);  /*maskは他の流儀もある*/
/*
stdin,stdout,stderr、に/dev/nullを割り当てておく
*/
  if(closeall) {fd=open("/dev/null",O_RDWR); dup(fd); dup(fd);}

/*
file lock、Appendix Dを参照
他にpidをフィアルに入れて多重起動を避ける等もある
*/
  if(ignsigcld)  /*子プロセスの面倒をみるか？*/
    setsigchild();
  daemonmode=1;
}

#ifdef __linux
#include <string.h>
static const char *__progname = "<unset_progname>";

void
setprogname(const char *progname)
{
  __progname = strrchr(progname, '/');
  if (__progname == NULL)
    __progname = progname;
  else
    __progname++;
}

const char *
getprogname(void)
{
  return __progname;
}
#endif



void usage(char *format,...)
{ va_list args;
  va_start(args,format);
  fprintf(stderr,"%s: ",getprogname());
  vfprintf(stderr,format,args);
  va_end(args);
}

