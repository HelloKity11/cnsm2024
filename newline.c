#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
int gpassiveTCP(char*,fd_set*);
void errexit(char*,...);
int writecmd(int,char*,int),readcmd(int,char*,int);
#define LINEBUF 1024
#define LINE 100
#define MGK 19-4
char bbuf[LINEBUF];

struct iovec IBUF[LINE];
char abuf[LINE][1024];
int cp= -1; //current message if cp==LINE, round up to 0
int wp=0; // if round up, wp=1

void downB(int ac) {
    int i;
    for(i=1; i<LINE; i++) {
        IBUF[i-1]=IBUF[i];
    }
    IBUF[LINE-1].iov_base=abuf[ac];
    IBUF[LINE-1].iov_len=strlen(abuf[ac]);
}

void addmsg(char *msg) {
    if(++cp >= LINE) {
        cp=0;
        wp=1;
    }

    if(!wp) {
        strcpy(abuf[cp],msg);
        IBUF[cp].iov_base=abuf[cp];
        IBUF[cp].iov_len=strlen(msg);
    } else {
        strcpy(abuf[cp],msg);
        downB(cp);
    }
}

void setN(int s,int n, int st) {
    char buf[1024],cbuf[8];
    int i;
    sprintf(buf,"%d",n);
    writecmd(s,buf,strlen(buf));
    buf[0]='\0';
    for(i=0; i<n; i++) {
        sprintf(cbuf,"%zu ",IBUF[i+st].iov_len);
        strcat(buf,cbuf);
    }
    writecmd(s,buf,strlen(buf));
}

void sendM(int s,int num) {
    if(!wp) {
        if(cp+1 <= num) {
            setN(s,cp+1,0);
            writev(s,IBUF,cp+1);
        } else {
            setN(s,num,cp-num+1);
            writev(s,&IBUF[cp-num+1],num);
        }
    } else {
        setN(s,num,LINE-num);
        writev(s,&IBUF[LINE-num],num);
    }
}

int main(int argc, char *argv[])
{   int ssd, alen, n, i, c, num, flag = 0;
    extern int daemonmode;
    (void)argc;
    struct sockaddr_storage fsin;
    char buf[LINEBUF];
    char *str;
    time_t timer;
    fd_set sfds, ssfds, rfds;
    struct timeval tv;
    setprogname(argv[0]);
    daemonmode=1;
    n = gpassiveTCP(argv[1], &sfds);
    timer = time(NULL);
    str = ctime(&timer);
    strncpy(buf, &str[4], MGK); buf[MGK]=' ';
    strcpy(&buf[MGK+1], "Start of dengon server.\n"); /*最初の伝言*/
    addmsg(buf);

    for(;;) {/*無限ループ*/
        ssfds = sfds;
        if(select(n + 1, &ssfds, NULL, NULL, NULL) < 0) continue;
        for(i = 0; i <= n; i++)
            if(FD_ISSET(i, &ssfds)) {
                alen = sizeof(fsin);
                ssd = accept(i, (struct sockaddr*)&fsin, (socklen_t*)&alen);
                if(ssd < 0) errexit("accept failed: %s\n", strerror(errno));
                readcmd(ssd, buf, sizeof(buf)); num = atoi(buf); if(num>LINE)num=LINE; if(num<0)num=1;
                // most recent num message write. not old 
                sendM(ssd, num); /*伝言数を最大num行書く*/
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
                timer = time(NULL);
                str = ctime(&timer);
                strncpy(bbuf,&str[4],MGK); bbuf[MGK]=' ';
                strcpy(&bbuf[MGK+1],buf);
                addmsg(bbuf);
                close(ssd);
            }
    }
    return 0; //not reached
}
