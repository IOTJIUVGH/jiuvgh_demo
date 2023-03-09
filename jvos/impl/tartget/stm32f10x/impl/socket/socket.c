/**
 ******************************************************************************
 * @file    mxos_socket.c
 * @author  William Xu
 * @version V1.0.0
 * @date    05-Aug-2018
 * @brief   This file provide the MXOS Socket abstract layer convert functions.
 ******************************************************************************
 *
 *  UNPUBLISHED PROPRIETARY SOURCE CODE
 *  Copyright (c) 2016 MXCHIP Inc.
 *
 *  The contents of this file may not be disclosed to third parties, copied or
 *  duplicated in any form, in whole or in part, without the prior written
 *  permission of MXCHIP Corporation.
 ******************************************************************************
 */


#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "merr.h"
#include "mxos_socket.h"

/******************************************************
 *                      Macros
 ******************************************************/

#ifdef inet_addr
#undef inet_addr
#endif


#ifdef inet_ntoa
#undef inet_ntoa
#endif

struct in_addr in_addr_any = {INADDR_ANY};
#if MXOS_CONFIG_IPV6
struct in6_addr in6_addr_any = IN6ADDR_ANY_INIT;
#endif

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Variables Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

int socket(int domain, int type, int protocol)
{
    return lwip_socket( domain, type, protocol );
}

int setsockopt (int socket, int level, int optname, void *optval, socklen_t optlen)
{
    return lwip_setsockopt( socket, level, optname, optval, optlen );
}

int getsockopt (int socket, int level, int optname, void *optval, socklen_t *optlen_ptr)
{
    return lwip_getsockopt( socket, level, optname, optval, optlen_ptr );
}

int bind (int socket, struct sockaddr *addr, socklen_t length)
{
    return lwip_bind ( socket, addr, length);
}

int connect (int socket, struct sockaddr *addr, socklen_t length)
{
    return lwip_connect( socket, addr, length );
}

int listen (int socket, int n)
{
    return lwip_listen( socket, n );
}

int accept (int socket, struct sockaddr *addr, socklen_t *length_ptr)
{
    return lwip_accept( socket, addr, length_ptr );
}

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    return lwip_select( nfds, readfds, writefds, exceptfds, timeout );
}

#ifndef CONFIG_MX108
int ioctl(int s, int cmd, ...)
{
    va_list ap;
    va_start( ap, cmd );
    void *para = va_arg( ap, void *);
    va_end( ap );
    return lwip_ioctl(s, cmd, para);
}
#endif

int poll(struct pollfd *fds, int nfds, int timeout)
{
	int maxfd=0;
	int i, n;
	fd_set rfds, wfds, efds;
	struct timeval t;
	int ret = 0, got;

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&efds);
	for(i=0; i<nfds; i++) {
		if (fds[i].fd > maxfd)
			maxfd = fds[i].fd;
		if (fds[i].events & (POLLIN|POLLPRI))
			FD_SET(fds[i].fd, &rfds); 
		if (fds[i].events & (POLLOUT))
			FD_SET(fds[i].fd, &wfds); 
		if (fds[i].events & (POLLERR|POLLHUP|POLLNVAL))
			FD_SET(fds[i].fd, &efds); 
		fds[i].revents = 0;
	}
	if (timeout < 0) {
		n = lwip_select(maxfd+1, &rfds, &wfds, &efds, NULL);
	} else {
		t.tv_sec = timeout / 1000;
		t.tv_usec = (timeout % 1000) * 1000;
		n = lwip_select(maxfd+1, &rfds, &wfds, &efds, &t);
	}
	if (n <= 0)
		return n;
	for(i=0; i<nfds; i++) {
		got=0;
		if (FD_ISSET(fds[i].fd, &rfds)) {
			fds[i].revents = fds[i].events & (POLLIN|POLLPRI);
			got = 1;
		}
		if (FD_ISSET(fds[i].fd, &wfds)) {
			fds[i].revents = fds[i].events & POLLOUT;
			got = 1;
		}
		if (FD_ISSET(fds[i].fd, &efds)) {
			fds[i].revents = fds[i].events & (POLLERR|POLLHUP|POLLNVAL);
			got = 1;
		}
		if (got == 1) {
			ret++;
		}
	}

	return ret;
}

int send (int socket, const void *buffer, size_t size, int flags)
{
    return lwip_send( socket, buffer, size, flags );
}

int sendto (int socket, const void *buffer, size_t size, int flags, const struct sockaddr *addr, socklen_t length)
{
    return lwip_sendto( socket, buffer, size, flags, addr, length);
}

int recv (int socket, void *buffer, size_t size, int flags)
{
    return lwip_recv( socket, buffer, size, flags );
}

int recvfrom (int socket, void *buffer, size_t size, int flags, struct sockaddr *addr, socklen_t *length_ptr)
{
    return lwip_recvfrom( socket, buffer, size, flags, addr, length_ptr );
}

ssize_t read (int filedes, void *buffer, size_t size)
{
    return lwip_recv(filedes, buffer, size, 0);
}

ssize_t write (int filedes, const void *buffer, size_t size)
{
    return lwip_send(filedes, buffer, size, 0);
}

int close (int filedes)
{
    return lwip_close( filedes );
}

int shutdown(int s, int how)
{
	return lwip_shutdown(s, how);
}

struct hostent * gethostbyname (const char *name)
{
    return lwip_gethostbyname( name );
}

int getaddrinfo(const char *nodename,
       const char *servname,
       const struct addrinfo *hints,
       struct addrinfo **res)
{
	return lwip_getaddrinfo(nodename,servname,hints,res);
}

void freeaddrinfo(struct addrinfo *ai)
{
	lwip_freeaddrinfo(ai);
}
int getpeername (int s, struct sockaddr *name, socklen_t *namelen)
{
	return lwip_getpeername (s, name, namelen);
}
int getsockname (int s, struct sockaddr *name, socklen_t *namelen)
{
	return lwip_getsockname (s, name, namelen);
}

uint32_t inet_addr (const char *name)
{
    return ipaddr_addr( name );
}


char *inet_ntoa (struct in_addr addr)
{
    return ip4addr_ntoa(  &(addr) );
}

#if LWIP_IPV6
#ifdef inet_ntop
#undef inet_ntop
#endif

char * inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
    return (((af) == AF_INET6) ? ip6addr_ntoa_r((const ip6_addr_t*)(src),(dst),(size)) \
     : (((af) == AF_INET) ? ip4addr_ntoa_r((const ip4_addr_t*)(src),(dst),(size)) : NULL));
}

#else

const char * inet_ntop(int af, const void *cp, char *buf, socklen_t len)
{
    struct in_addr addr;
    char *addr_str;

    if (af != AF_INET)
        return NULL;

    
    addr.s_addr = *(uint32_t*)cp;;
    addr_str = inet_ntoa(addr);
    
    strcpy(buf, addr_str);
    return addr_str;
}

int inet_pton(int af, const char *cp, void *buf)
{
    uint32_t addr;
    
    if (af != AF_INET)
        return 0;

    return ip4addr_aton(cp, buf);
}

#endif

