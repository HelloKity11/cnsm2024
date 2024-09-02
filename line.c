#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
int gpassiveTCP();
void errexit();
int writecmd();
#define LINEBUF 1024
#define LINE 100
#define MGK 19-4
char abuf[LINE][LINEBUF];

int main(int argc, char *argv[])
{   int ssd, alen, n, i, c, k, flag = 0;
    int ac = 0, wa = 0; // wrap around
    (void)argc;
    struct sockaddr_storage fsin;
    char buf[LINEBUF];
    char *str;
    time_t timer;
    fd_set sfds, ssfds, rfds;
    struct timeval tv;
    setprogname(argv[0]);
    n = gpassiveTCP(argv[1], &sfds);
    timer = time(NULL);
    str = ctime(&timer);
    strncpy(buf,&str[4],MGK); buf[MGK]=' ';
    strcpy(&buf[MGK+1], "Start of dengon server.\n"); /*最初の伝言*/
    strcpy(abuf[0],buf);
    for(;;) {/*無限ループ*/
        ssfds = sfds;
        if(select(n + 1, &ssfds, NULL, NULL, NULL) < 0) continue;
        for(i = 0; i <= n; i++)
            if(FD_ISSET(i, &ssfds)) {
                alen = sizeof(fsin);
                ssd = accept(i, (struct sockaddr*)&fsin, (socklen_t*)&alen);
                if(ssd < 0) errexit("accept failed: %s\n", strerror(errno));
                if(!wa) {
                    char cbuf[8];
                    sprintf(cbuf,"%d",ac+1);
                    writecmd(ssd,cbuf,strlen(cbuf)); /*伝言数を１行書く*/
                    for(k=0; k<=ac; k++)
                        writecmd(ssd, abuf[k], strlen(abuf[k])); /*現在の伝言を書き出す*/
                } else {
                    char cbuf[8];
                    sprintf(cbuf,"%d",LINE);
                    writecmd(ssd,cbuf,strlen(cbuf)); /*伝言数を１行書く*/
                    for(k=ac+1; k<LINE; k++)
                        writecmd(ssd, abuf[k], strlen(abuf[k])); /*現在の伝言を書き出す*/
                    for(k=0; k<=ac; k++)
                        writecmd(ssd, abuf[k], strlen(abuf[k])); /*現在の伝言を書き出す*/
                }
                FD_ZERO(&rfds);
                FD_SET(ssd, &rfds);
                tv.tv_sec = 30; /* 30 sec timer */
                tv.tv_usec = 0;
                select(ssd + 1, &rfds, NULL, NULL, &tv); /*in 30sec*/
                if(FD_ISSET(ssd, &rfds)) {
                    c = recv(ssd, buf, sizeof(buf) - 1, flag); /*新たな伝言を読み込む*/
                    if(c < 0) strcpy(buf, "recv error.\n");
                    else if(c == 0) strcpy(buf, "no data.\n");
                    else buf[c] = '\0';
                }
                else { strcpy(buf, "timeout in 30sec.\n"); }
                if(++ac >= LINE) { ac = 0; wa = 1; } // once wa=1, it always 1.
                timer = time(NULL);
                str = ctime(&timer);
                strncpy(abuf[ac],&str[4],MGK); abuf[ac][MGK]=' ';
                strcpy(&abuf[ac][MGK+1],buf);
                close(ssd);
            }
    }
    return 0; //not reached
}
