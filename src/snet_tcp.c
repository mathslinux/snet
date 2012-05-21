/**
 * @file   snet_tcp.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Sun May 13 12:09:31 2012
 * 
 * @brief  Tcp wrapper
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>
#include "snet.h"

static int snet_read(STcpClientSocket *st, void *buf, int buflen,
                     SnetError *err);
static void snet_async_read(STcpClientSocket *st, SReadFunc cb);
static int snet_write(STcpClientSocket *st, void *buf, int buflen,
                      SnetError *err);

static STcpClientSocket *__snet_tcp_client_socket_new(SInetAddr *ia)
{
    SInetAddrEntry *ia_entry;

    int sockfd;
    int ret = -1;
    LIST_FOREACH(ia_entry, &ia->ia_head, entries) {
        struct sockaddr *s;
        sockfd = socket(ia_entry->ai_family, SOCK_STREAM, 0);
        if (sockfd < 0) {
            printf ("socket() failed\n");
            return NULL;
        }

        s = (struct sockaddr *)(&ia_entry->sa);
        ret = connect(sockfd, s, sizeof(struct sockaddr_in));
        if (ret == 0) {
            break;
        } else {
            close(sockfd);
            continue;
        }
    }

    /* Failed */
    if (ret) {
        return NULL;
    }

    STcpClientSocket *st = malloc(sizeof(*st));
    if (!st) {
        close(sockfd);
        return NULL;
    } else {
        memset(st, 0, sizeof(*st));
    }

    st->sockfd = sockfd;
    memcpy(&st->ia, ia_entry, sizeof(st->ia));
    return st;
}

/** 
 * 
 * 
 * @param hostname 
 * @param port 
 * 
 * @return 
 */
STcpClientSocket *snet_tcp_client_socket_new(const char *hostname, int port)
{
    /**
     * 1. Create a ia
     * 2. Create a STcp
     * 3. Set ia to s.ia
     * 4. Connect to socket to s.ia
     */

    STcpClientSocket *s = NULL;
    SInetAddr *ia = NULL;

    ia = snet_inet_addr_new(hostname, port);
    if (!ia) {
        printf ("Create internet address failed\n");
        return NULL;
    }

    s = __snet_tcp_client_socket_new(ia);
    if (s) {
        s->read = snet_read;
        s->async_read = snet_async_read;
        s->write = snet_write;
    }
    
    snet_inet_addr_free(ia);
    return s;
}

/** 
 * Free a TCP client socket
 * 
 * @param s 
 */
void snet_tcp_client_socket_free(STcpClientSocket *s)
{
    if (!s)
        return ;

    close(s->sockfd);
    free(s);
}

/** 
  * Read socket, NB: this is block function, for async, use watch* helper first.
 * 
 * @param buf Buffer to write to
 * @param buflen Error code, if everything is ok, it will be SNET_IO_OK,
 *        if no data need to read, it will be SNET_IO_EOF. For details,
 *        see @SnetError
 * @param err 
 * 
 * @return The number of bytes read
 */
static int snet_read(STcpClientSocket *st, void *buf, int buflen,
                     SnetError *err)
{
    if (!buf || buflen <= 0 || !err) {
        goto failed;
    }
    int count;

    count = read(st->sockfd, (char *)buf, buflen);
//    count = read(st->sockfd, buf, buflen);

    if (count < 0) {
        goto failed;
    } else if (count == 0) {
        *err = SNET_IO_EOF;
    } else {
        *err = SNET_IO_OK;
    }
    return count;
    
failed:
    if (err)
        *err = SNET_IO_ERROR;
    return -1;
}

typedef struct {
    STcpClientSocket *st;
    SReadFunc cb;
} AsyncReadParameter;
    
static void *async_read_thread(void *data)
{
    AsyncReadParameter *p = (AsyncReadParameter *)data;
    
    for ( ; ; ) {
        int ret;
        struct timeval timeout;
        fd_set fds;

        /* Set the timeout to 100ms, FIXME */
        timeout.tv_sec = 0;
        timeout.tv_usec = 1;
        FD_ZERO(&fds); 
        FD_SET(p->st->sockfd, &fds);

        ret = select(p->st->sockfd + 1, &fds, NULL, NULL, &timeout);
        if (ret == 0) {
            continue;
        } else if (ret < 0) {
            printf ("select() failed: %s\n", strerror(errno));
            goto thread_quit;
        }

        /* Process data */
        if (p->cb(p->st, NULL) == 0) {
            /* data has been read done, quit */
            goto thread_quit;
        }
    }

thread_quit:
    free(p);
    pthread_exit(NULL);
}

/** 
 * Read socket asynchronously
 * 
 * @param st STcpClientSocket instance
 * @param cb the function to call when data canbe read
 */
static void snet_async_read(STcpClientSocket *st, SReadFunc cb)
{
    if (!cb)
        return ;

    pthread_t pth;
    pthread_attr_t attr;
    AsyncReadParameter *p;

    p = malloc(sizeof(*p));
    if (!p)
        return ;
    p->st = st;
    p->cb = cb;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&pth, &attr, async_read_thread, p);
}

/** 
  * Write buffer to socket
 * 
 * @param buf Buffer to read from
 * @param buflen Error code, if everything is ok, it will be SNET_IO_OK,
 *        if nothing was written, it will be SNET_IO_EOF. For details,
 *        see @SnetError
 * @param err 
 * 
 * @return The number of bytes written
 */
static int snet_write(STcpClientSocket *st, void *buf, int buflen,
                     SnetError *err)
{
    if (!buf || buflen <= 0 || !err) {
        goto failed;
    }
    int count;

    count = write(st->sockfd, buf, buflen);
    
    if (count < 0) {
        goto failed;
    } else if (count == 0) {
        *err = SNET_IO_OK;
    } else {
        *err = SNET_IO_OK;
    }
    return count;
    
failed:
    if (err)
        *err = SNET_IO_ERROR;
    return -1;
}

#if 0
static int read_cb(STcpClientSocket *st, void *data)
{
    char buf[1024] = {"123"};
    SnetError err;
    st->read(st, buf, sizeof(buf), &err);
    printf ("buf: %s, err is %d\n", buf, err);
    
    return 0;
}

int main(int argc, char *argv[])
{
    STcpClientSocket *st = snet_tcp_client_socket_new("192.168.1.200", 80);

    if (st) {
        char *teststr = "I am a test app\n";
        SnetError err; 
        st->write(st, teststr, sizeof(teststr), &err);
        /* st->read(st, buf, sizeof(buf), &err); */
        /* printf ("buf: %s, err is %d\n", buf, err); */
        st->async_read(st, read_cb);
        
        pause();
        snet_tcp_client_socket_free(st);
    }
    return 0;
}
#endif 
