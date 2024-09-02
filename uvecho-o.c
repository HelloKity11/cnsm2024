#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

uv_tcp_t server;
uv_tcp_t server1;
uv_loop_t * loop;

uv_connection_cb connection_cb(uv_stream_t * server, int status);
uv_read_cb read_cb(uv_stream_t * stream, ssize_t nread, uv_buf_t *buf);
uv_alloc_cb alloc_buffer(uv_handle_t * handle, size_t size, uv_buf_t* buf);
int uv_shutdown(uv_shutdown_t* req, uv_stream_t* stream, uv_shutdown_cb cb);


uv_alloc_cb alloc_buffer(uv_handle_t * handle, size_t size, uv_buf_t * buf)
{
    (void)handle;
    buf->base = malloc(size);
    buf->len = size;
    return (uv_alloc_cb)0;
}

static void on_close(uv_handle_t* peer) {
    free(peer);
}

uv_shutdown_cb  after_shutdown(uv_shutdown_t* req, int status) {
    (void)status;
    uv_close((uv_handle_t*) req->handle, on_close);
    free(req);
    return (uv_shutdown_cb)0;
}
int  close_cb_called=0;

uv_close_cb close_cb(uv_handle_t* handle) {
    (void)handle;
    close_cb_called++;
    return (uv_close_cb)0;
}


static void after_write(uv_write_t* req, int status) {
    if (status != 0) {
        fprintf(stderr, "write error %s\n", uv_err_name(status));
        uv_close((uv_handle_t*)req->handle, (uv_close_cb)close_cb);
        free(req);
        return;
    }
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
    r = uv_listen((uv_stream_t *) &server1, 128,
                  (uv_connection_cb) connection_cb);

    return uv_run(loop, UV_RUN_DEFAULT);
}

uv_connection_cb connection_cb(uv_stream_t * server, int status)
{
    uv_tcp_t * client = malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);
    uv_accept(server, (uv_stream_t *) client);
    int r = uv_read_start((uv_stream_t *) client, (uv_alloc_cb) alloc_buffer,
                          (uv_read_cb) read_cb);
    (void)r;
    (void)status;
    return (uv_connection_cb)0;
}

void copy_buffer(int size,uv_buf_t * buf,uv_buf_t *buf0)
{
    buf->base = malloc(size);
    buf->len = size;
    memcpy(buf->base, buf0->base,size);
}

uv_read_cb read_cb(uv_stream_t * stream, ssize_t nread, uv_buf_t *buf)
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
        uv_shutdown(sreq, stream, (uv_shutdown_cb)after_shutdown);
    }
    else if(nread==0) {;} //nothing to do. or do shutdown?
    else {
        uv_write_t * req = (uv_write_t *) malloc(sizeof(uv_write_t));
        uv_buf_t *nbuf= (uv_buf_t*)malloc(sizeof(uv_buf_t));
        copy_buffer(nread,nbuf,buf);
        int r = uv_write(req, stream, nbuf, 1, after_write);
        //who and when free nbuf->base, nbuf ??
        if(r<0) 	fprintf(stderr, "write error\n");
    }
    return (uv_read_cb) 0;
}

