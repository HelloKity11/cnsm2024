#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
int gpassiveTCP();
void errexit();
void print_sockaddr();

#ifndef FD_COPY
#define FD_COPY(f, t)   (void)(*(t) = *(f))
#endif

#define LINEBUF 4096
#define DEBUG 1

struct ThreadArgs {int clntSock;};
void *ThreadMain(void *arg);

int main(int argc,char *argv[])
{ struct sockaddr_storage fsin;
  int ms, ss, alen, i;
  fd_set rfds, afds;
  pthread_t threadID;
  struct ThreadArgs *threadArgs;

  (void)argc;
  setprogname(argv[0]);
  ms = gpassiveTCP(argv[1], &afds);
#ifdef DEBUG
  printf("main pid=%d, pthread_id=%ld\n", getpid(), (long int)pthread_self());
#endif
  while(1){
    FD_COPY(&afds,&rfds);
    if(select(ms + 1, &rfds, (fd_set*)0, (fd_set*)0, (struct timeval*)0) < 0) {
      printf("errno=%d\n", errno);
      errexit("select: %s\n", strerror(errno)); }
    for(i = 0; i <= ms; i++) {
      if(FD_ISSET(i, &afds) && FD_ISSET(i, &rfds)) {
      alen = sizeof(fsin);
      if((ss = accept(i,(struct sockaddr*)&fsin, (socklen_t*)&alen)) < 0)
        errexit("accept failed: %s\n", strerror(errno));
#ifdef DEBUG
      printf("client connected from ");
      print_sockaddr(&fsin, alen);
#endif
      if ((threadArgs = (struct ThreadArgs *) 
	     malloc(sizeof(struct ThreadArgs)))  == NULL)
	      errexit("malloc faild");
      threadArgs -> clntSock = ss;
      if (pthread_create(&threadID, NULL, ThreadMain, 
			 (void *) threadArgs) != 0) // スレッド作成
	      errexit("ptread_create faild");
      }
    }
  }
}

void *ThreadMain(void *threadArgs)
{
  int fd, cc, flag = 0;
  char buf[LINEBUF];

  pthread_detach(pthread_self()); //スレッドは勝手に終了して良い．
  fd = ((struct ThreadArgs *) threadArgs) -> clntSock;
  free(threadArgs);
#ifdef DEBUG
  printf("pid=%d, pthread_id=%ld\n", getpid(), (long int)pthread_self());
#endif
  while(1) {
        if((cc = recv(fd, buf, sizeof(buf), flag)) < 0)
          errexit("echo read: %s\n", strerror(errno));
        if(cc && send(fd, buf, cc, flag) < 0)
          errexit("echo write: %s\n", strerror(errno));
        if(cc == 0){
          close(fd); return 0;
        }
  }
}
