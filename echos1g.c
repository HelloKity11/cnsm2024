#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define LINEBUF 4096
typedef void (*sighandler_t)(int);
int gpassiveTCP();
void errexit();

int main(int argc, char *argv[])
{
  return 0; //not reached
}

