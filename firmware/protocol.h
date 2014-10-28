/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

/* Avalon4 protocol package type */
#define MM_VERSION_LEN	15
#define AVA4_H1	'A'
#define AVA4_H2	'V'

#define AVA4_P_COINBASE_SIZE	(6 * 1024 + 64)
#define AVA4_P_MERKLES_COUNT	30

#define AVA4_P_COUNT	40
#define AVA4_P_DATA_LEN 32

#define AVA4_P_DETECT	10
#define AVA4_P_STATIC	11
#define AVA4_P_JOB_ID	12
#define AVA4_P_COINBASE	13
#define AVA4_P_MERKLES	14
#define AVA4_P_HEADER	15
#define AVA4_P_POLLING	16
#define AVA4_P_TARGET	17
#define AVA4_P_REQUIRE	18
#define AVA4_P_SET	19
#define AVA4_P_TEST	20

#define AVA4_P_NONCE		23
#define AVA4_P_STATUS		24
#define AVA4_P_ACKDETECT	25
#define AVA4_P_TEST_RET		26

#define AVA4_MODULE_BROADCAST	0
#endif	/* _PROTOCOL_H_ */
