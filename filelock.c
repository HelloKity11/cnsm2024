#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifndef BSD
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#else
#include <sys/file.h>
#endif

#define SEQFILE "seqno"
#define MAXBUF 100
#ifndef NOLOCK

#endif
void sys_err(char *s){fprintf(stderr,"%s\n", s); exit(1);}

#ifdef NOLOCK
void my_lock(int fd){(void)fd;return;}
void my_unlock(int fd){(void)fd;return;}
#else
#ifdef BSD
void my_lock(int fd)
{if(flock(fd, LOCK_EX) == -1) sys_err("can't LOCK_EX");}
void my_unlock(int fd)
{if(flock(fd, LOCK_UN) == -1) sys_err("can't LOCK_UN");}
#else
void my_lock(int fd)
{struct flock arg;
 arg.l_type = F_WRLCK; arg.l_whence = 0; 
 arg.l_start = 0; arg.l_len = 0;
 if(fcntl(fd, F_SETLKW, &arg) == -1) sys_err("can't LOCK_EX");}
void my_unlock(int fd)
{struct flock arg;
 arg.l_type = F_UNLCK; arg.l_whence = 0;
 arg.l_start = 0; arg.l_len = 0;
 if(fcntl(fd, F_SETLKW, &arg) == -1) sys_err("can't LOCK_UN");}
#endif
#endif

int main(int argc, char *argv[])
{ int fd, i, n, pid, seqno;
  char buff[MAXBUF];
  (void)argc;(void)argv;

  pid = getpid();
  (void)pid;
  if((fd = open(SEQFILE, 2)) < 0)
    sys_err("can't open SEQFILE");
  for(i = 0; i < 1000; i++) {
    my_lock(fd);
    lseek(fd, 0L, 0);
    if((n = read(fd, buff, sizeof(buff) - 1)) <= 0)
      sys_err("read error");
    buff[n] = '\0';
    if((n = sscanf(buff, "%d\n", &seqno)) != 1)
      sys_err("sscanf error");
#ifdef DEBUG
    printf("pid = %d, seq = %d\n", pid, seqno);
#endif
    seqno++;
    sprintf(buff, "%04d\n", seqno);
    n = strlen(buff);
    lseek(fd, 0L, 0);
    if(write(fd, buff, n) != n)
      sys_err("write error");
    my_unlock(fd);
  }
  close(fd);
  return 0;
}
