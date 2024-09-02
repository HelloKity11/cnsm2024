#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
int gconnectTCP(char*,char*);
int readcmd(int,char*,int),writecmd(int,char*,int);
#define LINE 100
#define LINEBUF 1024
#define GNUM 10 // get 過去伝言数

struct iovec IBUF[LINE];
char abuf[LINE][1024];

void getmsg(int s) {
    int i,j,num;
    char buf[1024];
    char *cp;
    readcmd(s,buf,sizeof(buf));  // how many messages
    num=atoi(buf);
    readcmd(s,buf,sizeof(buf));  // length of each message separated by ' '
    cp=buf;

    for(i=0; i<num; i++) {
        IBUF[i].iov_base=abuf[i];
        IBUF[i].iov_len=j=atoi(cp);
        abuf[i][j]='\0'; // force 0 termination.
        cp=strchr(cp,' ');
        cp++;
    }
    readv(s,IBUF,num);
    for(i=0;i<num;i++) {printf("%s",(char*)IBUF[i].iov_base);usleep(500000);} //good value ?
}

int main(int argc, char *argv[])
{ int s, n, flag = 0, opt = 1, num = GNUM; (void)argc;
  char buf[LINEBUF];
  if(argc>3) num=atoi(argv[3]);
  setprogname(argv[0]);
  s = gconnectTCP(argv[1], argv[2]);
  sprintf(buf,"%d",num);
  writecmd(s,buf,strlen(buf));
  getmsg(s);
  fputs("Please write a new message within 30 seconds.\n", stdout); 
  fgets(buf, sizeof(buf), stdin); /*新伝言取り込み*/
  ioctl(s, FIONBIO, &opt);
  n = send(s, buf, strlen(buf), flag);
  if(n < 0) fputs("send error\n", stdout);
  return 0;
}
