#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define LINEBUF 4096
int gppassiveTCP();
void errexit();

#define MAXSOCK 10
char *progname;

void usage1() {
  printf("usage: %s -p port# [10000]\n", progname);
  exit(1);
}

int main(int argc, char *argv[])
{
  int ch;
  char *port = "10000";
  struct sockaddr_storage fsin;
  int             ms, ss, fd, alen, cc, i, j, n, flag = 0;
  struct pollfd   fds[MAXSOCK];
  char            buf[LINEBUF];

  progname = argv[0];
  while ((ch = getopt(argc, argv, "p:")) != -1) {
    switch (ch) {
    case 'p': port = optarg; break;
    case '?':
    default: usage1();
    }
  }
  argc -= optind; 
  argv += optind;

  bzero(&fds, sizeof(fds));
  n = ms = gppassiveTCP(port, &fds, MAXSOCK);
  for (i = 0; i < ms; i++)
    fds[i].events = POLLIN;
  while (1) {
    if (poll(fds, n, -1) == -1) {
      if (errno == EINTR) continue;
      errexit("select: %s\n", strerror(errno));
    }
    for (i = 0; i < ms; i++) {
      if (fds[i].revents & POLLIN) {
	alen = sizeof(fsin);
	if ((ss = accept(fds[i].fd, (struct sockaddr *) & fsin, 
			(socklen_t*)&alen)) < 0) {
	  if (errno == EINTR) continue;
	  errexit("accept failed: %s\n", strerror(errno));
	}
	if (n >= MAXSOCK) {
          fd = 0; /* no slot found */
	  for (j = ms; j < MAXSOCK; j++) {
	    if (fds[j].events == POLLNVAL) { /* reusable slot found */
	      n = MAXSOCK;	/* > never occure */
	      fds[j].fd = ss; fds[j].events = POLLIN;
              fd = 1 ; /* slot found mark */
	      break;
	    }
	  } 
	  if(!fd) close(ss); /* no slot */
	} else {
	  fds[n].fd = ss; fds[n++].events = POLLIN;
	}
      }
    }
    for (i = ms; i < n; i++)
      if (fds[i].revents & POLLIN) {
	if ((cc = recv(fds[i].fd, buf, LINEBUF, flag)) < 0)
	  errexit("echo read: %s\n", strerror(errno));
	if (cc && send(fds[i].fd, buf, cc, flag) < 0)
	  errexit("echo write: %s\n", strerror(errno));
	if (cc == 0) {
	  close(fds[i].fd);
	  fds[i].events = POLLNVAL;
	  if (i == n - 1) n--;
	}
      }
  }
  return 0; //not raeched
}
