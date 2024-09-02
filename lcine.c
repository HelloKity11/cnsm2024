#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
int gconnectTCP(char*,char*);
int readcmd();
#define LINEBUF 1024

int main(int argc, char *argv[])
{ int s, n, k,flag = 0, opt = 1; (void)argc;
  char buf[LINEBUF];
  setprogname(argv[0]);
  s = gconnectTCP(argv[1], argv[2]);
  n = readcmd(s, buf, sizeof(buf) - 1 ); /*伝言の数が１行目*/
  k = atoi(buf);
  while(k-->0) {
    n = readcmd(s, buf, sizeof(buf) - 1 ); /*伝言読み込み*/
    if(n < 0) strcpy(buf, "READ ERROR\n");
    else if(n == 0) strcpy(buf, "NO DATA\n");
    else buf[n] = '\0';
    fputs(buf, stdout);
}
  fputs("Please write a new message within 30 seconds.\n", stdout); 
  fgets(buf, sizeof(buf), stdin); /*新伝言取り込み*/
  ioctl(s, FIONBIO, &opt);
  n = send(s, buf, strlen(buf), flag);
  if(n < 0) fputs("send error\n", stdout);
  return 0;
}
