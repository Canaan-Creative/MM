/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include "system_config.h"
#include "io.h"
#include "i2c.h"

volatile unsigned int *twi_cr_reg = (unsigned int *)0x80000600 ; //TWI Ctrl Register
volatile unsigned int *twi_wd_reg = (unsigned int *)0x80000604 ; //TWI Ctrl Register
volatile unsigned int *twi_rd_reg = (unsigned int *)0x80000608 ; //TWI Ctrl Register

#define TWI_CR *twi_cr_reg //TWI Write Data
#define TWI_WD *twi_wd_reg //TWI Write Data
#define TWI_RD *twi_rd_reg //TWI Read Data

void twi_start( void ){
	TWI_CR = 0x03 ;
	while( (TWI_CR & 0x04) != 0x04);
}

void twi_wr( char buf ){
	TWI_WD = buf ;
	TWI_CR = 0x13 ;
	while( (TWI_CR & 0x04) != 0x04);
}

char twi_rd( void ){
	TWI_CR = 0x23 ;
	while( (TWI_CR & 0x04) != 0x04);
	return TWI_RD ;
}

void twi_stop( void ){
	TWI_CR = 0x33 ;
	while( (TWI_CR & 0x04) != 0x04);
}

void twi_write_2byte( unsigned int buf , char SLV_ADDR )
{
	twi_start()  ;
	twi_wr(SLV_ADDR<<1) ;//slave addr
	twi_wr(0x00) ;//register addr
	twi_wr(buf ) ;
	twi_wr(buf>>8 ) ;
	twi_stop() ;
}

unsigned int twi_read_2byte( char SLV_ADDR )
{
	char tmp ;
	twi_start()  ;
	twi_wr(SLV_ADDR<<1) ;//slave addr
	twi_wr(0x00) ;//register addr
	twi_start()  ;
	twi_wr((SLV_ADDR<<1)|0x1) ;//slave addr + read
	tmp = twi_rd() ;
	tmp = (twi_rd() <<8)|tmp;
	twi_stop()   ;
	return tmp ;
}

