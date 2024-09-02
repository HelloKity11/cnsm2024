#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
int passiveUDP();
void errexit();

int main(int argc, char *argv[])
{ struct sockaddr_in fsin;
  char *service = "time";
  char buf[1];
  int sock, alen;
  time_t now;

  switch(argc) {
  case 1: break;
  case 2: service = argv[1]; break;
  default: errexit("usage: UDPtimed [port]\n");
  }
  sock = passiveUDP(service);
  while(1) {
    alen = sizeof(fsin);
    if(recvfrom(sock, buf, sizeof(buf), 0,
		(struct sockaddr *)&fsin, (socklen_t*)&alen) < 0)
      errexit("recvfrom: %s\n", strerror(errno));
    (void)time(&now);
    now = htonl(now);
    (void)sendto(sock, (char*)&now, sizeof(now), 0,
		 (struct sockaddr*)&fsin, sizeof(fsin));
  }
}
