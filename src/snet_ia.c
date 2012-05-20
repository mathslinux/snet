/**
 * @file   snet_ia.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Sun May 13 19:26:55 2012
 * 
 * @brief  Internet Address Wrapper
 * 
 * 
 */

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>

#include "snet.h"


/**
 *  Creates a Internet Address from a host name and port.
 * 
 * @param hostname 
 * @param port 
 * 
 * @return 
 */
SInetAddr *snet_inet_addr_new(const char *hostname, int port)
{
    if (!hostname || port < 0) {
        printf("Invalid hostname or port\n");
        return NULL;
    }

    SInetAddr* ia = NULL;
    struct addrinfo hints;
    struct addrinfo *result = NULL, *curr;
    int ret;

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;

    ret = getaddrinfo(hostname, NULL, &hints, &result);
    if (ret) {
        printf("getaddrinfo: %s\n", strerror(errno));
        goto failed;
    }

	ia = malloc(sizeof(*ia));
	if (!ia) {
		printf("malloc:%s\n", strerror(errno));
		goto failed;
	} else {
		memset(ia, 0, sizeof(*ia));
		ia->hostname = strdup(hostname);
	}
	for (curr = result; curr != NULL; curr = curr->ai_next) {
		SInetAddrEntry *new = malloc(sizeof(*new));
		if (!new) {
			printf("malloc: %s\n", strerror(errno));
			goto failed;
		} else {
			memset(new, 0, sizeof(*new));
		}
		memcpy(&new->sa, curr->ai_addr, curr->ai_addrlen);
        new->ai_family = curr->ai_family;
		new->sa.sin_port = htons(port);
		LIST_INSERT_HEAD(&ia->ia_head, new, entries);
	}

	if (result) {
		freeaddrinfo(result);
	}
    
    return ia;

failed:
	if (result) {
		freeaddrinfo(result);
	}
	if (ia) {
		snet_inet_addr_free(ia);
	}
	return NULL;
}

/** 
 * Free internet address list 
 * 
 * @param ia 
 */
void snet_inet_addr_free(SInetAddr *ia)
{
	if (!ia)
		return ;

	SInetAddrEntry *ia_entry, *next;

	/* Free hostname */
	if (ia->hostname)
		free(ia->hostname);

	/* Free internet address list */
	LIST_FOREACH_SAFE(ia_entry, &ia->ia_head, entries, next) {
		LIST_REMOVE(ia_entry, entries);
		free(ia_entry);
    }

	free(ia);
}

#if 0
int main(int argc, char *argv[])
{
    SInetAddr *ia = snet_inet_addr_new("www.google.com", 80);
	SInetAddrEntry *ia_entry;

	if (!ia) {
		return -1;
	}
	
	LIST_FOREACH(ia_entry, &ia->ia_head, entries) {
		printf("%s\n", inet_ntoa(ia_entry->sa.sin_addr));
    }

	snet_inet_addr_free(ia);
	
    return 0;
}
#endif 
