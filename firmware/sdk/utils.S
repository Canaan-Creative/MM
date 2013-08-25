	.section    .text, "ax", @progbits        
	.global     irq_enable, irq_disable, irq_set_mask, irq_get_mask
	.global     jump, halt
	.global     get_sp, get_gp

irq_enable:
	mvi     r1, 1
	wcsr	IE, r1
	ret

irq_disable:
	mvi     r1, 0
	wcsr	IE, r1
	ret

irq_set_mask:
	wcsr	IM, r1
	ret

irq_get_mask:
	rcsr    r1, IM
	ret
	
jump:
	b       r1 

halt:
	bi halt

get_sp:
	mv      r1, sp
	ret

get_gp:
	mv      r1, gp
	ret       
