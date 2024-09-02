#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
int connectUDP();
void errexit();

int main(int argc, char *argv[])
{ char *service = "time";
  char buf[1];
  int sock;
  time_t now;

  switch(argc) {
  case 2: break;
  case 3: service = argv[2]; break;
  default: errexit("usage: UDPtime host [port]\n");
  }
  sock = connectUDP(argv[1], service);
  send(sock, buf, 0, 0); /* 0 byte send */
  recv(sock, &now, sizeof(now), 0); /* get 1 long */
  now = ntohl(now);
  printf("%s", ctime(&now));
  return 0;
}
