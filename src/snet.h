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

typedef struct _STCPClientSocket STCPClientSocket;

struct _STCPClientSocket {
    int sockfd;
    SInetAddr *ia;
};


/** 
 * Create a TCP Client socket with hostname and port
 * 
 * @param hostname 
 * @param port 
 * 
 * @return 
 */
STCPClientSocket *snet_tcp_client_socket_new(const char *hostname, int port);

/** 
 * Free a TCP client socket
 * 
 * @param s 
 */
void snet_tcp_client_socket_free(STCPClientSocket *s);

#endif
