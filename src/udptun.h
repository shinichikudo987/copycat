/**
 * \file udptun.h
 * \brief This contains the argument structures and macros.
 * \author k.edeline
 * \version 0.1
 */

#ifndef _UDPTUN_MAIN_H
#define _UDPTUN_MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <argp.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>

/** TODO: config file
 * \def __BUFFSIZE
 * \brief The size of the client and server buffers.
 */
#define __BUFFSIZE 8192

/**
 * \def __CLOSE_TIMEOUT
 * \brief The time to wait for delayed finack/ack while closing 
 *  a connection (sec).
 */
#define __CLOSE_TIMEOUT 1

/** 
 * \struct arguments
 *	\brief The programs arguments.
 */
struct arguments {
   enum { CLI_MODE, SERV_MODE, FULLMESH_MODE, NONE_MODE } mode; /*!<  The tunnelling mode. */
   uint8_t verbose;      /*!<  verbose mode */
   uint8_t silent;       /*!<  silent mode */
   uint8_t planetlab;    /*!<  PlanetLab mode */
   uint8_t freebsd;      /*!<  FREEBSD mode */

   char *udp_daddr;  /*!<  The UDP destination address. */
   char *tcp_daddr;  /*!< The TCP destination address. */
   char *tcp_saddr;  /*!<  The TCP source address. */
   uint16_t udp_dport;  /*!<  The UDP destination port. */
   uint16_t udp_sport;  /*!<   The UDP source port. */
   uint16_t udp_lport;  /*!<  The UDP listen port. */
   uint16_t tcp_dport;  /*!<  The TCP destination port. */
   uint16_t tcp_sport;  /*!<  The TCP source port. */

   char *config_file;   /*!< The destination file  */
   char *dest_file;  /*!< The destination file  */
   char *if_name;    /*!< The tun interface name */
   uint8_t inactivity_timeout; /*!< The inactivity timeout */
};

#include "debug.h"
#include "sock.h"
#include "cli.h"
#include "serv.h"
#include "peer.h"

/**
 * \def max(a,b)
 * \brief max macro with type checking.
 */
#define max(a,b) \
   __extension__({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })


#endif

