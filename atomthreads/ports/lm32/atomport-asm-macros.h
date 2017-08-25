#ifndef __ATOMPORT_ASM_MACROS_H_
#define __ATOMPORT_ASM_MACROS_H_


/* Define Register Macs Value */
/* Register Width */
#define WORD_SIZE 	4

/* Save Register */
#define r1_IDX		0		/* Index 0 */
#define r2_IDX		1		/* Index 4 */
#define r3_IDX		2		/* Index 8 */
#define r4_IDX		3		/* Index 12 */
#define r5_IDX		4		/* Index 16 */
#define r6_IDX		5		/* Index 20 */
#define r7_IDX		6		/* Index 24 */
#define r8_IDX		7		/* Index 28 */
#define r9_IDX		8		/* Index 32 */
#define r10_IDX		9		/* Index 36 */
/* R26 Global Poniter (gp) */
#define gp_IDX		10		/* Index 40 */
/* R27 Frame Poniter (fp) */
#define fp_IDX		11		/* Index 44 */
/* R28 Stack Poniter (sp) */
#define sp_IDX		12		/* Index 48 */
/* R29 Return Address (ra) */
#define ra_IDX		13		/* Index 52 */
/* R30 Exception Address (ea) */
#define ea_IDX		14		/* Index 54 */
/* R31 Breakponit Address (ba) */
#define ba_IDX		15		/* Index 58 */
/* PC Program Counter */
#define pc_IDX		16		/* Index 62 */

#define NUM_CTX_REGS 	17


#endif /* __ATOMPORT_ASM_MACROS_H_ */
