/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */
#ifndef _TWI_H_
#define _TWI_H_
void twi_write_2byte( unsigned int buf , char SLV_ADDR );
unsigned int twi_read_2byte( char SLV_ADDR );
#endif
