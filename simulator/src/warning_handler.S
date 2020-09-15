.file "warning_handler.S"

#include <msp430.h>

; take the CPU into an endless loop
.global elkCurMode
.global elkClearMark

.balign 2
.global	    Port4_Warning_handler	;port 4 interrupt
.section    __interrupt_vector_32,"ax",@progbits
.word	    Port4_Warning_handler
.text
.type	    Port4_Warning_handler, @function
Port4_Warning_handler:
    NOP
    DINT
    NOP

    MOV.W	#0, &P4IFG
    MOV.B  	#1, &elkCurMode
    MOV.B   #1, &elkClearMark

    NOP
    EINT
    NOP

    RETI
.size	Port4_Warning_handler, .-Port4_Warning_handler
