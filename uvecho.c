#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

uv_tcp_t server;
uv_tcp_t server1;
uv_loop_t * loop;

void connection_cb(uv_stream_t * server, int status);
void read_cb(uv_stream_t * stream, ssize_t nread, uv_buf_t *buf);
void alloc_buffer(uv_handle_t * handle, size_t size, uv_buf_t* buf);
int uv_shutdown(uv_shutdown_t* req, uv_stream_t* stream, uv_shutdown_cb cb);

void alloc_buffer(uv_handle_t * handle, size_t size, uv_buf_t * buf)
{
    (void)handle;
    //fprintf(stderr,"alloc buf size=%zu\n",size);
    *buf =  uv_buf_init((char*) malloc(size), size);
}

static void on_close(uv_handle_t* peer) {
    free(peer);
}

void  after_shutdown(uv_shutdown_t* req, int status) {
    (void)status;
    uv_close((uv_handle_t*) req->handle, on_close);
    free(req);
}

int  close_cb_called=0;

void close_cb(uv_handle_t* handle) {
    (void)handle;
    close_cb_called++;
}

int main(void)
{
    struct sockaddr_in addr ;
    struct sockaddr_in6 addr6 ;
    loop = uv_default_loop();
    uv_ip4_addr("0.0.0.0", 12345, &addr);
    uv_ip6_addr("::", 12345, &addr6);
    uv_tcp_init(loop, &server);
    uv_tcp_init(loop, &server1);
    uv_tcp_bind(&server, (const struct sockaddr*) &addr, 0);
    uv_tcp_bind(&server1, (const struct sockaddr*) &addr6, 0);
    int r = uv_listen((uv_stream_t *) &server, 128,
                      (uv_connection_cb) connection_cb);
    if (r) {
        fprintf(stderr, "IPv4 Listen error %s\n", uv_strerror(r));
        return 1;
    }
    r = uv_listen((uv_stream_t *) &server1, 128,
                  (uv_connection_cb) connection_cb);
    if (r) {
        fprintf(stderr, "IPv6 Listen error %s\n", uv_strerror(r));
        return 1;
    }
    return uv_run(loop, UV_RUN_DEFAULT);
}

void connection_cb(uv_stream_t * server, int status)
{
    if(status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        // error!
        return;
    }
    uv_tcp_t * client = malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);
    if(uv_accept(server, (uv_stream_t *) client) == 0) {
        uv_read_start((uv_stream_t *) client, (uv_alloc_cb) alloc_buffer,
                      (uv_read_cb) read_cb);
    }
    else {
        uv_close((uv_handle_t*) client, on_close);
    }
}

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

void free_write_req(uv_write_t *req) {
    write_req_t *wr = (write_req_t*) req;
    free(wr->buf.base);
    free(wr);
}

void on_file_write(uv_write_t *req, int status) {
    if(status) fprintf(stderr, "Write error %s\n", uv_strerror(status));
    free_write_req(req);
}

void read_cb(uv_stream_t * stream, ssize_t nread, uv_buf_t *buf)
{
    if (nread < 0) {
        uv_shutdown_t* sreq = malloc(sizeof *sreq);
        if (buf->base) free(buf->base);
        if (nread == UV_EOF) {
            //	fprintf(stderr, "EOF(%s)\n", uv_err_name(nread));
            ; // nothing to tell
        } else if (nread == UV_ECONNRESET) {
            fprintf(stderr, "read error RST(%s)\n", uv_err_name(nread));
        } else {
            fprintf(stderr, "read error(%s)\n", uv_err_name(nread));
        }
        uv_shutdown(sreq, stream, (uv_shutdown_cb) after_shutdown);
    }
    else if(nread==0) {
        if (buf->base) free(buf->base);
    } else {
        write_req_t * req = (write_req_t *) malloc(sizeof(write_req_t));
        req->buf = uv_buf_init(buf->base,nread);
        (void) uv_write((uv_write_t*)req, stream, &req->buf, 1, (uv_write_cb) on_file_write);
    }
}
