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

#include "snet.h"

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

#if 0
int main(int argc, char *argv[])
{
    STcpClientSocket *st = snet_tcp_client_socket_new("www.google.com", 80);

    if (st)
        snet_tcp_client_socket_free(st);
    return 0;
}
#endif 
