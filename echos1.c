#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
int passiveTCP();
void errexit();
#define QLEN 5
#define LINEBUF 4096
typedef void (*sighandler_t)(int);

int main(int argc, char *argv[])
{
  return 0;
}


