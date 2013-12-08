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

#define AVA2_P_COINBASE_SIZE	(2 * 1024)
#define AVA2_P_MERKLES_COUNT	20

#define AVA2_P_COUNT	41
#define AVA2_P_DATA_LEN		(41 - 9)

#define AVA2_P_DETECT	10
#define AVA2_P_STATIC	11
#define AVA2_P_JOB_ID	12
#define AVA2_P_COINBASE	13
#define AVA2_P_MERKLES	14
#define AVA2_P_HEADER	15

#define AVA2_P_ACK		21
#define AVA2_P_NAK		22
#define AVA2_P_NONCE		23
#define AVA2_P_HEARTBEAT	24
#define AVA2_P_ACKDETECT	25

#endif	/* _PROTOCOL_H_ */
