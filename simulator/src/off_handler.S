.file "off_handler.S"

#include <msp430.h>

; take the CPU into an endless loop

.balign 2
.global	    Port5_OFF_handler	;port 5 interrupt
.section    __interrupt_vector_27,"ax",@progbits
.word	    Port5_OFF_handler
.text
.type	    Port5_OFF_handler, @function
Port5_OFF_handler:
    NOP
    DINT
    NOP

    MOV.W	#0, &P5IFG
    ;MOV.W   SP-4, SP

    NOP
    EINT
    NOP

    JMP .               ;endless loop here.
    RETI
.size	Port5_OFF_handler, .-Port5_OFF_handler
