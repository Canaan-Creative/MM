/*
 * Copyright (c) 2010, Atomthreads Project. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. No personal names or organizations' names associated with the
 *    Atomthreads project may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE ATOMTHREADS PROJECT AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "atomport-asm-macros.h"

.section .text


/**
 * @brief Return threads switch 
 * Exit critical area and Enable interrupt 
 * Return func call
 */
.globl contextExitCritical
contextExitCritical:
    	/* Enalbe interrupt return func call */
	ori r15, r15, 0x0001
    	wcsr IE, r15
    	nop
    	nop
    	nop
    	ret

/**
 * @brief Enter threads switch
 * Enter critical area and Disable interrupt
 * Return func all
 */
.globl contextEnterCritical
contextEnterCritical:
    	/* Disable interrupt, direct to writing r0 to IE */
    	wcsr IE, r0
    	nop
    	nop
    	nop
    	ret

/**
 * Function that performs the contextSwitch. Whether its a voluntary release
 * of CPU by thread or a pre-emption, under both conditions this function is
 * called. The signature is as follows:
 *
 * archContextSwitch(ATOM_TCB *old_tcb, ATOM_TCB *new_tcb)
 */
.globl archContextSwitch
archContextSwitch:
    	/* Disable global interrupt */
    	wcsr IE, r0

    	/* R1 is argument0, save old_tcb when we return from here */
    	/* R9 and R10 use to temp memory */
    	mv r9, r1
    	lw r10, (r9+0)
    
    	mvhi r9, r1_IDX
    	sli r9, r9, 2
    	add r9, r10, r9
    	/* Save r3 */
    	sw (r9+8), r3
    	/* Save r4 */
   	sw (r9+12), r4 
    	/* Save r5 */
    	sw (r9+16), r5 
    	/* Save r6 */
    	sw (r9+20), r6 
    	/* Save r7 */
    	sw (r9+24), r7
    	/* Save r8 */
    	sw (r9+28), r8
    	/* Save gp */
    	sw (r9+40), gp 
    	/* Save fp */
    	sw (r9+44), fp 
    	/* Save sp */
    	sw (r9+48), sp
    	/* Save ra */
    	sw (r9+52), ra

    	/* R2 is argument1, load new_tcb data to memory */
    	/* R11 and R12 use to temp memory */
    	mv r11, r2
    	lw r12, (r11+0)
   
    	mvhi r11, r1_IDX
    	sli r11, r11, 2
    	add r11, r12, r11
    	/* Load r3 */
    	lw r3, (r11+8)
    	/* Load r4 */
    	lw r4, (r11+12)
    	/* Load r5 */
    	lw r5, (r11+16)
    	/* Load r6 */
    	lw r6, (r11+20)
    	/* Load r7 */
    	lw r7, (r11+24)
    	/* Load r8 */
    	lw r8, (r11+28)
    	/* Load gp */
    	lw gp, (r11+40)
    	/* Load fp */
    	lw fp, (r11+44)
    	/* Load sp */
    	lw sp, (r11+48)
    	/* Load ra */
    	lw ra, (r11+52)
    	nop
    	nop
    	nop
   	/* Enable global interrupts */
   	ori r15, r15, 0x0001
   	wcsr IE, r15
   	nop
   	nop
   	nop
   	/* Return call function */
   	ret

/**
 * archFirstThreadRestore(ATOM_TCB *new_tcb)
 *
 * This function is responsible for restoring and starting the first
 * thread the OS runs. It expects to find the thread context exactly
 * as it would be if a context save had previously taken place on it.
 * The only real difference between this and the archContextSwitch()
 * routine is that there is no previous thread for which context must
 * be saved.
 *
 * The final action this function must do is to restore interrupts.
 */
.globl archFirstThreadRestore
archFirstThreadRestore:
    	/* Disable global interrupts */
    	wcsr IE, r0

    	/* R1 is argument0, load first thread */
    	mv r13, r1
    	lw r14, (r13+0)

    	mvhi r13, r1_IDX
    	sli r13, r13, 2
    	add r13, r14, r13    
    	/* Load r3 */
    	lw r3, (r13+8)
    	/* Load r4 */
    	lw r4, (r13+12)
    	/* Load r5 */
    	lw r5, (r13+16)
    	/* Load r6 */
    	lw r6, (r13+20)
    	/* Load r7 */
    	lw r7, (r13+24)
    	/* Load r8 */
    	lw r8, (r13+28)
    	/* Load gp */
    	lw gp, (r13+40)
    	/* Load fp */
    	lw fp, (r13+44)
    	/* Load sp */
    	lw sp, (r13+48)
    	/* Load ra */
    	lw ra, (r13+52)
    	nop
    	nop
    	nop
    	/* Enable global interrupts */
    	ori r15, r15, 0x0001
    	wcsr IE, r15
    	nop
    	nop
    	nop
    	/* Return call function */
    	ret
