#ifndef TINC_NET_H
#define TINC_NET_H

/*
    net.h -- header for net.c
    Copyright (C) 1998-2005 Ivo Timmermans
                  2000-2016 Guus Sliepen <guus@tinc-vpn.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "system.h"

#include "ipv6.h"
#include "cipher.h"
#include "digest.h"
#include "event.h"

#define EPOLL_MAX_EVENTS_PER_LOOP 32

#ifdef ENABLE_JUMBOGRAMS
#define MTU 9018        /* 9000 bytes payload + 14 bytes ethernet header + 4 bytes VLAN tag */
#else
#define MTU 1518        /* 1500 bytes payload + 14 bytes ethernet header + 4 bytes VLAN tag */
#endif

#define MINMTU 512      /* Below this we don't consider UDP to be working */

/* MAXSIZE is the maximum size of an encapsulated packet: MTU + seqno + srcid + dstid + padding + HMAC + compressor overhead */
#define MAXSIZE (MTU + 4 + sizeof(node_id_t) + sizeof(node_id_t) + CIPHER_MAX_BLOCK_SIZE + DIGEST_MAX_SIZE + MTU/64 + 20)

/* MAXBUFSIZE is the maximum size of a request: enough for a MAXSIZEd packet or a 8192 bits RSA key */
#define MAXBUFSIZE ((MAXSIZE > 2048 ? MAXSIZE : 2048) + 128)

#define MAXSOCKETS 8    /* Probably overkill... */

typedef struct mac_t {
	uint8_t x[6];
} mac_t;

typedef struct ipv4_t {
	uint8_t x[4];
} ipv4_t;

typedef struct ipv6_t {
	uint16_t x[8];
} ipv6_t;

typedef struct node_id_t {
	uint8_t x[6];
} node_id_t;

typedef uint16_t length_t;
typedef uint32_t seqno_t;

#define AF_UNKNOWN 255

struct sockaddr_unknown {
	uint16_t family;
	uint16_t pad1;
	uint32_t pad2;
	char *address;
	char *port;
};

typedef union sockaddr_t {
	struct sockaddr sa;
	struct sockaddr_in in;
	struct sockaddr_in6 in6;
	struct sockaddr_unknown unknown;
} sockaddr_t;

#ifdef SA_LEN
#define SALEN(s) SA_LEN(&s)
#else
#define SALEN(s) (s.sa_family==AF_INET?sizeof(struct sockaddr_in):sizeof(struct sockaddr_in6))
#endif

#define SEQNO(x) ((x)->data + (x)->offset - 4)
#define SRCID(x) ((node_id_t *)((x)->data + (x)->offset - 6))
#define DSTID(x) ((node_id_t *)((x)->data + (x)->offset - 12))
#define DATA(x) ((x)->data + (x)->offset)
#define DEFAULT_PACKET_OFFSET 12

typedef struct vpn_packet_t {
	length_t len;           /* The actual number of valid bytes in the `data' field (including seqno or dstid/srcid) */
	length_t offset;        /* Offset in the buffer where the packet data starts (righter after seqno or dstid/srcid) */
	int priority;           /* priority or TOS */
	uint8_t data[MAXSIZE];
} vpn_packet_t;

/* Packet types when using SPTPS */

#define PKT_COMPRESSED 1
#define PKT_MAC 2
#define PKT_PROBE 4

typedef struct listen_socket_t {
	io_t tcp;
	io_t udp;
	sockaddr_t sa;
	bool bindto;
	int priority;
} listen_socket_t;

#include "conf.h"
#include "list.h"

typedef struct outgoing_t {
	struct node_t *node;
	int timeout;
	timeout_t ev;
} outgoing_t;

typedef struct ports_t {
	char *tcp;
	char *udp;
} ports_t;

extern list_t outgoing_list;

extern int maxoutbufsize;
extern int seconds_till_retry;
extern int addressfamily;
extern unsigned replaywin;
extern bool localdiscovery;

extern bool udp_discovery;
extern int udp_discovery_keepalive_interval;
extern int udp_discovery_interval;
extern int udp_discovery_timeout;

extern int mtu_info_interval;
extern int udp_info_interval;

extern listen_socket_t listen_socket[MAXSOCKETS];
extern int listen_sockets;
extern io_t unix_socket;
extern int keylifetime;
extern int udp_rcvbuf;
extern int udp_sndbuf;
extern bool udp_rcvbuf_warnings;
extern bool udp_sndbuf_warnings;
extern int max_connection_burst;
extern int fwmark;
extern bool do_prune;
extern ports_t myport;
extern bool device_standby;
extern bool autoconnect;
extern bool disablebuggypeers;
extern int contradicting_add_edge;
extern int contradicting_del_edge;
extern time_t last_config_check;

extern char *proxyhost;
extern char *proxyport;
extern char *proxyuser;
extern char *proxypass;
typedef enum proxytype_t {
	PROXY_NONE = 0,
	PROXY_SOCKS4,
	PROXY_SOCKS4A,
	PROXY_SOCKS5,
	PROXY_HTTP,
	PROXY_EXEC,
} proxytype_t;
extern proxytype_t proxytype;

extern char *scriptinterpreter;
extern char *scriptextension;

/* Yes, very strange placement indeed, but otherwise the typedefs get all tangled up */
#include "connection.h"
#include "node.h"

extern void retry_outgoing(outgoing_t *outgoing);
extern void handle_incoming_vpn_data(void *data, int flags);
extern void finish_connecting(struct connection_t *c);
extern bool do_outgoing_connection(struct outgoing_t *outgoing);
extern void handle_new_meta_connection(void *data, int flags);
extern void handle_new_unix_connection(void *data, int flags);
extern int setup_listen_socket(const sockaddr_t *sa);
extern int setup_vpn_in_socket(const sockaddr_t *sa);
extern bool send_sptps_data(struct node_t *to, struct node_t *from, int type, const void *data, size_t len);
extern bool receive_sptps_record(void *handle, uint8_t type, const void *data, uint16_t len);
extern void send_packet(struct node_t *n, vpn_packet_t *packet);
extern void receive_tcppacket(struct connection_t *c, const char *buffer, size_t length);
extern bool receive_tcppacket_sptps(struct connection_t *c, const char *buffer, size_t length);
extern void broadcast_packet(const struct node_t *n, vpn_packet_t *packet);
extern char *get_name(void);
extern void device_enable(void);
extern void device_disable(void);
extern bool setup_myself_reloadable(void);
extern bool setup_network(void);
extern void setup_outgoing_connection(struct outgoing_t *outgoing, bool verbose);
extern void try_outgoing_connections(void);
extern void close_network_connections(void);
extern int main_loop(void);
extern void terminate_connection(struct connection_t *c, bool report);
extern bool node_read_ecdsa_public_key(struct node_t *n);
extern void handle_device_data(void *data, int flags);
extern void handle_meta_connection_data(struct connection_t *c);
extern void regenerate_key(void);
extern void purge(void);
extern void retry(void);
extern int reload_configuration(void);
extern void load_all_nodes(void);
extern void try_tx(struct node_t *n, bool mtu);
extern void tarpit(int fd);

#ifndef HAVE_WINDOWS
#define closesocket(s) close(s)
#endif

#endif
