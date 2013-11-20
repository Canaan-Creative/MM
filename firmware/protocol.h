/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

/* Avalon2 protocol package type */
#define AVA2_H1	'A'
#define AVA2_H2	'V'

#define AVA2_T1	'O'
#define AVA2_T2	'N'

#define AVA2_P_COUNT		41
#define AVA2_P_DETECT	0
#define AVA2_P_STATIC	1
#define AVA2_P_JOB_ID	2
#define AVA2_P_COINBASE	3
#define AVA2_P_MERKLES	4

#define AVA2_P_ACK		5
#define AVA2_P_NAK		6
#define AVA2_P_NONCE		7
#define AVA2_P_HEARTBEAT	8
#define AVA2_P_ACKDETECT	9

#endif	/* _PROTOCOL_H_ */
