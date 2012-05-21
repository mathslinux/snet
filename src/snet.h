/**
 * @file   snet.h
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Sun May 13 23:30:10 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef SNET_H
#define SNET_H

#include "queue.h"

typedef enum {
    FALSE,
    TRUE,
} Bool;

/**
 * Internet Address wrapper
 * 
 */
typedef struct _SInetAddrEntry
{
    struct sockaddr_in sa;
    int ai_family;
    LIST_ENTRY(_SInetAddrEntry) entries;
} SInetAddrEntry;

typedef struct _SInetAddr
{
    char *hostname;
    int port;
    LIST_HEAD(, _SInetAddrEntry) ia_head;
} SInetAddr;

/** 
 *  Creates a Internet Address from a host name and port.
 * 
 * @param hostname 
 * @param port 
 * 
 * @return 
 */
SInetAddr *snet_inet_addr_new(const char *hostname, int port);

/** 
 * Free internet address list 
 * 
 * @param ia 
 * 
 * @return 
 */
void snet_inet_addr_free(SInetAddr *ia);

/**
 * TCP Client wrapper
 * 
 */

typedef enum {
    /* Code for socket read/write */
    SNET_IO_OK,                 /**< Everything is ok */
    SNET_IO_ERROR,              /**< Something error */
    SNET_IO_EOF,                /**< No data need to process */
} SnetError;

typedef struct _STcpClientSocket STcpClientSocket;

/* Socket read function, if this function return 0, stop watching
 * this socket, else continue watching whether socket is readable */
typedef int (*SReadFunc)(STcpClientSocket *st, void *data);

struct _STcpClientSocket {
    int sockfd;
    SInetAddr ia;

    /**
     * Read socket, NB: this is block function, for async, see watch* helper.
     * @param  st  STcpClientSocket instance
     * @param  buf Buffer to write to
     * @param  err Error code, if everything is ok, it will be SNET_IO_OK,
     *         if no data need to read, it will be SNET_IO_EOF. For details,
     *         see @SnetError
     *
     * @return The number of bytes read
     * 
     */
    
    int (*read)(STcpClientSocket *st, void *buf, int buflen, SnetError *err);

    /** 
     * Read socket asynchronously
     * 
     * @param st STcpClientSocket instance
     * @param cb the function to call when data canbe read
     *
     */
    void (*async_read)(STcpClientSocket *st, SReadFunc cb);
    
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
    int (*write)(STcpClientSocket *st, void *buf, int buflen, SnetError *err);
};


/** 
 * Create a TCP Client socket with hostname and port
 * 
 * @param hostname 
 * @param port 
 * 
 * @return 
 */
STcpClientSocket *snet_tcp_client_socket_new(const char *hostname, int port);

/** 
 * Free a TCP client socket
 * 
 * @param s 
 */
void snet_tcp_client_socket_free(STcpClientSocket *s);

#endif
