#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <openssl/ssl.h>
#include <openssl/err.h>


#define BUF_LEN 256

struct URL {
    char host[BUF_LEN];
    char path[BUF_LEN];
    char query[BUF_LEN];
    char fragment[BUF_LEN];
    unsigned short port;
};

void parseURL(const char *urlStr, struct URL *url, char **error) {
    char host_path[BUF_LEN];

    if (strlen(urlStr) > BUF_LEN - 1) {
        *error = "URL too long.\n";
        return;
    }

    // start from http://
    // sscanf succeed
    // string follows http://
    if (strncmp(urlStr, "https://",8)==0              &&
        sscanf(urlStr, "https://%s", host_path) ){

        char *p = NULL;

        p = strchr(host_path, '#');
        if (p != NULL) {
            strcpy(url->fragment, p);
            *p = '\0';
        }

        p = strchr(host_path, '?');
        if (p != NULL) {
            strcpy(url->query, p);
            *p = '\0';
        }

        p = strchr(host_path, '/');
        if (p != NULL) {
            strcpy(url->path, p);
            *p = '\0';
        }

        strcpy(url->host, host_path);

        // : found in host part
        p = strchr(url->host, ':');
        if (p != NULL) {
            // get port number
            url->port = atoi(p + 1);

            // atoi() fail or 0
            // port is 80
            if (url->port <= 0) {
                url->port = 443;
            }

            // send with null
            *p = '\0';
        }
        else {
            url->port = 443;
        }
    }
    else {
        *error = "URL must be https://host/path \n";
        return;
    }
// printf("query=%s\n",url->query);
}

int main(int argc, char **argv)
{
  int mysocket;
  int gconnectTCP();

  SSL *ssl;
  SSL_CTX *ctx;

  char msg[100];
  struct URL url;

    if (argc > 1) {
        char *error = NULL;
        parseURL(argv[1], &url, &error);

        if (error) {
            printf("%s\n", error);
            return 1;
        }
    } else return 1;

//    printf("getting http://%s%s%s\n\n", url.host, url.path, url.query);

  char *host = url.host;
  char *path = url.path;
  char port[8] ;
  sprintf(port,"%hu",url.port);

  mysocket = gconnectTCP(host,port); 

  SSL_load_error_strings();
  SSL_library_init();

  ctx = SSL_CTX_new(SSLv23_client_method());
  ssl = SSL_new(ctx);
  SSL_set_fd(ssl, mysocket);
  SSL_connect(ssl);

  sprintf(
    msg, 
    "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n",
    path,
    host
  );

  SSL_write(ssl, msg, strlen(msg));

  int buf_size = 256;
  char buf[buf_size];
  int read_size;

  do {
    read_size = SSL_read(ssl, buf, buf_size);
    printf("%s", buf);
  } while (read_size > 0);

  SSL_shutdown(ssl); 

  SSL_free(ssl); 
  SSL_CTX_free(ctx);
  ERR_free_strings();

  close(mysocket);

  return EXIT_SUCCESS;
}
