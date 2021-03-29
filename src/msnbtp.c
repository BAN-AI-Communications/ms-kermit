/* File MSNBTP.C
 * Bootp requestor
 *
 * Copyright (C) 1991, University of Waterloo.
 *	Copyright (C) 1982, 1997, Trustees of Columbia University in the 
 *	City of New York.  The MS-DOS Kermit software may not be, in whole 
 *	or in part, licensed or sold for profit as a software product itself,
 *	nor may it be included in or distributed with commercial products
 *	or otherwise distributed by commercial concerns to their clients 
 *	or customers without written permission of the Office of Kermit 
 *	Development and Distribution, Columbia University.  This copyright 
 *	notice must not be removed, altered, or obscured.
 *
 * Original version created by Erick Engelke of the University of
 *  Waterloo, Waterloo, Ontario, Canada.
 * Rewritten and extended for MS-DOS Kermit by Joe R. Doupnik, 
 *  Utah State University, jrd@cc.usu.edu, jrd@usu.Bitnet.
 *
 * Last edit
 * 12 Aug 1996 v3.15
 *
 *   BOOTP - Boot and DHCP Protocols, RFCs 951, 1048, 1395, 1531, 1541, 1533
 *   and successors 2131 and 2132.
 */

#include "msntcp.h"
#include "msnlib.h"

/*
 * structure for send and receives
 */
typedef struct bootp {
	byte	 bp_op;		/* packet op code / message type. */
	byte	 bp_htype;	/* hardware address type, 1 = Ethernet */
	byte	 bp_hlen;	/* hardware address len, eg '6' for Ethernet*/
	byte	 bp_hops;	/* client sets to zero, optionally used by
				   gateways in cross-gateway booting. */
	longword bp_xid;	/* transaction ID, a random number */
	word	 bp_secs;	/* filled in by client, seconds elapsed since
				   client started trying to boot. */
	word	 bp_flags;	/* DHCP flags */
	longword bp_ciaddr;	/* client IP address filled in by client */
				/*  if known */
	longword bp_yiaddr;	/* 'your' (client) IP address
				   filled by server if client doesn't know */
	longword bp_siaddr;	/* server IP address returned in bootreply */
	longword bp_giaddr;	/* gateway IP address,
				   used in optional cross-gateway booting. */
	byte	 bp_chaddr[16];	/* client hardware address, filled by client */
	byte	 bp_sname[64];	/* optional server host name, null terminated*/

	byte	 bp_file[128];	/* boot file name, null terminated string
				   'generic' name or null in bootrequest,
				   fully qualified directory-path
				   name in bootreply. */
	byte	 bp_vend[64+248]; /* 64 vendor-specific area + 248 DHCP */
};

/* UDP port numbers, server and client */
#define	IPPORT_BOOTPS	67
#define	IPPORT_BOOTPC	68

/* bootp.bp_op */
#define BOOTREQUEST 	1
#define BOOTREPLY	2

/* DHCP values from RFC 1531, RFC 1533 et seq. */
/* Command codes, option type 53, single octet of data */
#define	DHCPDISCOVER   1
#define	DHCPOFFER      2
#define	DHCPREQUEST    3
#define	DHCPDECLINE    4
#define	DHCPACK	       5
#define	DHCPNAK	       6
#define	DHCPRELEASE    7
#define DHCPRENEWING	100

/* DHCP command code, 53 decimal */
#define DHCP_COMMAND	53
#define OPTION_SERVERID 54
#define OPTION_END	255

#define VM_RFC1048   0x63538263L	/* magic cookie for BOOTP */
#define BOOTPTIMEOUT 30			/* seconds timeout to do bootup */

static longword DHCP_server_IP;		/* IP of DHCP server */
static long DHCP_lease, DHCP_renewal, DHCP_rebind;
static byte DHCP_state;
static longword xid;
static use_RFC2131;			/* Use RFC2131 REQUESTs */
static longword master_timeout;		/* hard shutdown grace interval */

/* global variables */
longword bootphost = 0xffffffffL;	/* broadcast IP */
byte hostname[MAX_STRING+1] = {0};	/* our fully qualified IP name */
extern word arp_hardware, MAC_len;	/* media details from msnpdi.asm */
extern byte kdomain[];			/* our IP domain string */
extern byte kbtpserver[];		/* IP of responding server */
extern	eth_address eth_addr;		/* six byte array */
extern	byte kdebug;
extern	byte bootmethod;
int request_busy = 0;			/* DHCP request() lock */

static int request(void);		/* send request, decode reply */
static void decode(struct bootp *, int);	/* decode Options */
static int notdhcp(struct bootp *, int);	/* detect non-DHCP pkt */

/*
 * do_bootp - Do Bootp or DHCP negotiations.
 *             returns 0 on success and sets ip address
 */

int 
do_bootp()
{
	outs("\r\n Requesting a ");
	if (bootmethod == BOOT_BOOTP)
		outs("Bootp server ");
	else	outs("DHCP server ");

	bootphost = 0xffffffffL;	/* broadcast IP */
	my_ip_addr = 0L;		/* init our IP address to unknown */
	DHCP_server_IP = 0L;		/* DHCP server IP address, 0 = none */
	DHCP_lease = 0L;		/* no lease expiration */
	DHCP_state = DHCPDISCOVER;	/* discover a DHCP server */
	request_busy = 0;		/* request() lock, unlock it */
	master_timeout = 0;		/* kill hard shutdown timer */
	kbtpserver[0] = 0;		/* found server's IP address */
	xid = htonl(set_timeout(0));	/* set xid as tod in Bios ticks */
	if (request() == -1)		/* Bootp/DHCP request, get response */
		return (-1);		/* fail */

	if (DHCP_server_IP == 0L)	/* no DHCP response, use Bootp */
		{
		bootmethod = BOOT_BOOTP;
		return (my_ip_addr != 0? 0: -1); /* -1 for fail, 0 for succ */
		}
					/* DHCP negotiations, continued */
	DHCP_lease = 0L;		/* no lease expiration, yet */
	bootmethod = BOOT_DHCP;
	DHCP_state = DHCPREQUEST;	/* set conditions for request() */
	xid++;		/* change id tag so competing responses are ignored */
	use_RFC2131 = 1;		/* use revision RFC2131 of DHCP */
	if (request() == -1)		/* send/process DHCP REQUEST and ACK*/
		return (-1);		/* fail */
	if (DHCP_state == DHCPNAK	/* if Request refused */
		&& use_RFC2131)		/* and we used new style request */
		{
		use_RFC2131 = 0;	/* try again with RFC1541 style */
		DHCP_state = DHCPREQUEST; /* set conditions for request() */
		if (request() == -1)	/* send/process DHCP REQUEST and ACK*/
			return (-1);	/* fail */
		}

	use_RFC2131 = 1;		/* reset for next attempt */
	if (DHCP_state != DHCPACK)
		return (-1);		/* failure to negotiate DHCP */
	bootphost = DHCP_server_IP;	/* only now remember server */
	return (my_ip_addr != 0? 0: -1); /* -1 for failure, 0 for success */
}

/* Request Bootp/DHCP information and decode DHCP ACK */
/* Global int request_busy is non-zero to prevent calling this routine
   as a result of calling tcp_tick() within it.
*/
static int
request(void)
{
	udp_Socket bsock;
	struct bootp sendbootp;				/* outgoing data */
	struct bootp rcvbootp;				/* incoming data */
	struct bootp register * rbp, * sbp;
	longword sendtimeout, bootptmo;
	word magictimeout = 1;
	int reply_len;

	request_busy = 1;			/* lock out tcp_tick() */
	sbp = &sendbootp;			/* outgoing request */
	rbp = &rcvbootp;			/* incoming response */
	memset((byte *)sbp, 0, sizeof(struct bootp));
	memset((byte *)rbp, 0, sizeof(struct bootp));

	sbp->bp_op = BOOTREQUEST;
	sbp->bp_htype = (byte)(arp_hardware & 0xff); /* hardware type */
	bcopy(eth_addr, sbp->bp_chaddr, MAC_len); /* hardware address */
	sbp->bp_hlen = (byte) MAC_len;		/* length of MAC address */
	sbp->bp_xid = xid;			/* identifier, opaque */
	sbp->bp_ciaddr = htonl(my_ip_addr);	/* client IP identifier */
	*(long *)&sbp->bp_vend[0] = VM_RFC1048;	/* magic cookie longword */

	if (bootmethod == BOOT_DHCP)		/* DHCP details */
		{
		sbp->bp_vend[4] = DHCP_COMMAND;	/* option, DHCP command */
		sbp->bp_vend[5] = 1;		/* length of value */
		sbp->bp_vend[6] = DHCPREQUEST;	/* Request data */
		sbp->bp_vend[7] = OPTION_END;	/* end of Options */
		if (DHCP_state == DHCPDISCOVER)	/* if first probe */
			{
			sbp->bp_flags = htons(1); /* set DHCP Broadcast bit */
			sbp->bp_vend[6] = DHCPDISCOVER; /* DHCP server discov*/
			}
		if (DHCP_state == DHCPREQUEST)	/* if Request, not Renewal */
			{
			sbp->bp_vend[7] = OPTION_SERVERID; /* server id */
			sbp->bp_vend[8] = 4;		/* length of value */
			*(long *)&sbp->bp_vend[9] = htonl(DHCP_server_IP);
			sbp->bp_vend[13] = OPTION_END;	/* end of Options */
			if (use_RFC2131)	  /* if not using RFC1541 */
				{
				sbp->bp_ciaddr = 0; /* no client identifier */
				sbp->bp_vend[13] = 50;	/* Requested IP Addr*/
				sbp->bp_vend[14] = 4;	/* length of value */
						/* our IP address goes here */
				*(long *)&sbp->bp_vend[15] = htonl(my_ip_addr);
				sbp->bp_vend[19] = OPTION_END;
				}
			my_ip_addr = 0;     /* now forget IP from DISCOVER */
			}
		}

	if (udp_open(&bsock, IPPORT_BOOTPC, bootphost, IPPORT_BOOTPS) == 0)
		{
		request_busy = 0;		/* clear lock */
		sock_close(&bsock);
       		return (-1);
		}
	bootptmo = set_timeout(BOOTPTIMEOUT);
	magictimeout = 1;			/* one second */

    while (chk_timeout(bootptmo) != TIMED_OUT) /* repeat datagrams */
    	{	/* send only bootp length requests, accept DHCP replies */
	bsock.rdatalen = 0;			/* clear old received data */
	reply_len = 0;				/* no reply yet */
	sbp->bp_xid = xid++;			/* identifier, opaque */
	sock_write(&bsock, (byte *)sbp,	sizeof(struct bootp) - 248);
	if (bootmethod == BOOT_BOOTP || DHCP_state == DHCPDISCOVER)
		outs(".");			/* progress indicator */

	sendtimeout = set_timeout(magictimeout++);
	if (magictimeout > 8)
		magictimeout = 8;		/* truncate waits */

	while (chk_timeout(sendtimeout) != TIMED_OUT)
		{
		if (chkcon() != 0)			/* Control-C abort */
			{
			outs(" Canceled by user");
			sock_close(&bsock);
			request_busy = 0;		/* unlock access */
			return (-1);			/* fail */
			}

    		if (tcp_tick(&bsock) == 0)		/* read packets */
			{		/* major network error if UDP fails */
			outs(" Network troubles, quitting");
			sock_close(&bsock);
			request_busy = 0;		/* unlock access */
			return (-1);			/* fail */
			}

		reply_len = sock_fastread(&bsock, (byte *)rbp, 
							sizeof(struct bootp));
		bsock.rdatalen = 0;		/* clear old received data */

		if (reply_len < sizeof(struct bootp) - 248)
			continue;		/* reply is too short */

		if (rbp->bp_xid != sbp->bp_xid)	/* not our ident? */
               		continue;		/* not for us */

		if (rbp->bp_yiaddr == 0)	/* no IP address for us */
			if (bootmethod == BOOT_BOOTP) /* 0 for DHCP NAKs */
	               		continue;		/* not for us */

		if (*(long *)rbp->bp_vend != *(long *)sbp->bp_vend)
			continue;		/* magic cookie mismatch */

		if (bootmethod == BOOT_DHCP &&
 			DHCP_state != DHCPDISCOVER &&
			notdhcp(rbp, reply_len))
			continue;	/* not a required DHCP response */
		break;
		}		/* end of while (chk_timeout(sendtimeout) */

	if (reply_len == 0)
		continue;		/* no data yet, send again */

	decode(rbp, reply_len);		/* extract response data */

	if (my_ip_addr == 0L)		/* if first time through */
		{
		my_ip_addr = ntohl(rbp->bp_yiaddr); /* bootp header */
		ntoa(kbtpserver, ntohl(rbp->bp_siaddr));
		}
	break;
	}				/* end while (chk_timeout(bootptmo) */
	sock_close(&bsock);
	request_busy = 0;		/* unlock access */
	if (chk_timeout(bootptmo) == TIMED_OUT)
		return (-1);		/* fail */
	return (my_ip_addr != 0? 0: -1); /* -1 for fail, 0 for success */
}

/* Return non-zero if reply does not contains DHCP Command, else return 0 */

static int
notdhcp(struct bootp * rbp, int reply_len)
{
	byte *p, *q;

	p = &rbp->bp_vend[4];		/* Point just after magic cookie */
	q = &rbp->bp_op + reply_len;	/* end of all possible vendor data */

	while (*p != 255 && (q - p) > 0)
		switch(*p)
		{
                case 0: 		/* Nop Pad character */
                	p++;
                	break;
		case 53:		/* DHCP Command from server */
			return (0);
		case 255:		/* end of options */
			return(1);
		default:
		  	p += *(p+1) + 2; /* skip other options */
			break;
                  }
	return(1);
}

/* Decode Bootp/DHCP Options from received packet */
static void
decode(struct bootp * rbp, int reply_len)
{
	byte *p, *q;
	word len;
	longword tempip;

	p = &rbp->bp_vend[4];		/* Point just after magic cookie */
	q = &rbp->bp_op + reply_len;	/* end of all possible vendor data */

	while (*p != 255 && (q - p) > 0)
		switch(*p)
		{
                case 0: /* Nop Pad character */
                	p++;
                	break;
		case 1: /* Subnet Mask */
			sin_mask = ntohl(*(longword *)(&p[2]));
			p += *(p+1) + 2;
			break;
		case 3: /* gateways */
			for (len = 0; len < *(p+1); len += 4)
			  arp_add_gateway(NULL,ntohl(*(longword*)(&p[2+len])));
			p += *(p+1) + 2;
			break;
		case 6: /* Domain Name Servers (BIND) */
			for (len = 0; len < *(p+1); len += 4)
		    	add_server(&last_nameserver, MAX_NAMESERVERS,
			def_nameservers, ntohl(*(longword*)(&p[2 + len])));
			p += *(p+1) + 2;
			break;
		case 12: /* our hostname, hopefully complete */
			bcopyff(p+2, hostname, (int)(p[1] & 0xff));
			hostname[(int)(p[1] & 0xff)] = '\0';
			p += *(p+1) + 2;
			break;
		case 15: /* RFC-1395, Domain Name tag */
			bcopyff(p+2, kdomain, (int)(p[1] & 0xff));
			kdomain[(int)(p[1] & 0xff)] = '\0';
			p += *(p+1) + 2;
			break;

		case 51:	/* DHCP Offer lease time, seconds */
			if (p[1] == 4)
				{
				DHCP_lease = ntohl(*(longword*)(&p[2]));
				if (DHCP_lease == -1L)	/* -1 is infinite */
					DHCP_lease = 0;	/* no timeout */
				else
					{
					if (DHCP_lease > 0x0ffffL)
						DHCP_lease = 0x0ffffL;
					DHCP_lease = set_timeout((int)(0xffff 
						& DHCP_lease));
					}
				}
			p += *(p+1) + 2;
			break;

		case 53:	/* DHCP Command from server */
			DHCP_state = p[2];	/* Command, to local state */
			p += *(p+1) + 2;
			break;
		case 54:	/* DHCP server IP address */
			if (p[1] == 4)
				if (tempip = *(longword*)(&p[2]))
					DHCP_server_IP = ntohl(tempip);
			p += *(p+1) + 2;
			break;
		case 58:	/* DHCP lease renewal time (T1) */
			if (p[1] == 4)
				{
				DHCP_renewal = ntohl(*(longword*)(&p[2]));
				if (DHCP_renewal == -1L)
					DHCP_renewal = 0;	/* no timeout */
				else
				     {
				     if (DHCP_renewal > 0x0ffffL)
						DHCP_lease = 0x0ffffL;
				     DHCP_renewal = set_timeout((int)(0xffff & 
						DHCP_renewal));
				     }
				}
			p += *(p+1) + 2;
			break;
		case 59:	/* DHCP rebind time (T2) */
			if (p[1] == 4)
				{
				DHCP_rebind = ntohl(*(long*)(&p[2]));
				if (DHCP_rebind > 0x0ffffL)
					DHCP_rebind = 0x0ffffL;
				DHCP_lease = DHCP_rebind =
				set_timeout((int)(0xffff & DHCP_rebind));
				}

			p += *(p+1) + 2;
			break;

		case 255:	/* end of options */
			break;
		default:
		  	p += *(p+1) + 2;
			break;
                  } 			/* end of switch */
}

/* Release DHCP granted IP information. Skip if DHCP ACK has not been
   received or if lease time is infinite (to help Novell's DHCP v2.0
   from clobbering itself with erasure of permanent assignments).
*/
void
end_bootp(void)
{
	udp_Socket bsock;
	struct bootp sendbootp;				/* outgoing data */
	struct bootp register * sbp;
	longword wait;

	if (DHCP_state != DHCPACK)
		return;				/* not using DHCP */

	if (DHCP_lease == 0)			/* infinite lease */
		return;

	sbp = &sendbootp;
	memset((byte *)sbp, 0, sizeof(struct bootp));

	udp_open(&bsock, IPPORT_BOOTPC, bootphost, IPPORT_BOOTPS);
	sbp->bp_op = BOOTREQUEST;
	sbp->bp_htype = (byte)(arp_hardware & 0xff);
	bcopy(eth_addr, sbp->bp_chaddr, MAC_len);
	sbp->bp_hlen = (byte) MAC_len;		/* length of MAC address */
	sbp->bp_xid = xid;
	*(long *)&sbp->bp_vend[0] = VM_RFC1048;	/* magic cookie longword */
	sbp->bp_vend[4] = DHCP_COMMAND;	/* option, DHCP command */
	sbp->bp_vend[5] = 1;		/* length of value */
	sbp->bp_vend[6] = DHCPRELEASE;	/* value, release DHCP server */
	sbp->bp_vend[7] = OPTION_SERVERID; /* server identification */
	sbp->bp_vend[8] = 4;		/* length of IP address */
	*(long *)&sbp->bp_vend[9] = htonl(DHCP_server_IP);
	sbp->bp_vend[13] = OPTION_END;	/* end of options */

	sock_write(&bsock, (byte *)sbp, sizeof(struct bootp));
	wait = set_ttimeout(1);		/* one Bios clock tick */
	while (chk_timeout(wait) != TIMED_OUT) ;	/* pause */
	sock_write(&bsock, (byte *)sbp, sizeof(struct bootp)); /* repeat */
	sock_close(&bsock);

 	DHCP_server_IP = 0L;		/* DHCP server IP address, 0 = none */
	DHCP_lease = 0L;		/* no lease expiration */
	DHCP_state = DHCPDISCOVER;
	my_ip_addr = 0L;		/* lose our IP address too */
}

/* Renew DHCP lease on our IP address. Skip if lease is infinite and if
   renewal has not timed out. DHCP_renewal is Bios time of day when renewal
   is needed; DHCP_lease is Bios time of day of lease (0 means infinite).
*/
int
DHCP_refresh()
{
	longword temp;

	if (DHCP_lease == 0 ||			/* infinite lease */
		chk_timeout(DHCP_renewal) != TIMED_OUT)
		return (0);			/* nothing to do yet */
	if (master_timeout)			/* shutting down hard */
		if (chk_timeout(master_timeout) == TIMED_OUT)
			return (-1);		/* fail, shuts down stack */
		else
			return (0);		/* still in grace interval */

	if (request_busy)			/* don't reenter request() */
		return (0);			/* return success */
	DHCP_state = DHCPRENEWING;		/* set new state */
	if (request() != -1)			/* send/got DHCP info */
		if (DHCP_lease == 0 ||		/* infinite lease */
		chk_timeout(DHCP_renewal) != TIMED_OUT) /* lease renewed */
			return (0);		/* success, lease renewed */
	outs("\r\n Failed to renew DHCP IP address lease");
	outs("\r\n Shutting down TCP/IP system in ");
	temp = DHCP_lease - set_ttimeout(0);	/* ticks from now */
	temp = ourldiv(temp, 18);		/* ticks to seconds */
	outdec((word)temp & 0xffff);
	outs(" seconds!\7\r\n");
	master_timeout = DHCP_lease;		/* set master shutdown */
	return (0);				/* stay alive for now */
}
