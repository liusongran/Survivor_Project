.file "on_handler.S"

#include <msp430.h>
;#include <elk.h>

; Recovers from an SRAM checkpoint by recomputing and checking the CRC of the SRAM block and restoring the register file

; Define the top of SRAM. The linker script has been modified so that compiled code does not clobber information here

#define SRAM_BOTTOM		(0x3C00-4)

.global __scheduler_run
.global __elk_check_nv
.global __elk_backup

.global svIntervalNum
.global elkCurMode
.global svVrfiedBp
.global svMarkedBp
;Overwrite the reset vector
.global	Port3_ON_handler	;port 3 interrupt
.section	__interrupt_vector_33,"ax",@progbits
.word	Port3_ON_handler
.text
.type	Port3_ON_handler, @function

.global Port3_ON_handler
Port3_ON_handler:
    NOP
    DINT
    NOP

    MOV.W	#0, &P3IFG
    MOV.W   #SRAM_BOTTOM,   SP      ;fixme!
    CALLA   #__elk_check_nv
    CMP.W   R12,     #0
    JNZ     _verify_failed

	MOV.B	#0, elkCurMode
    MOV.B   #0, svIntervalNum
    MOV.W   #0, svVrfiedBp
    MOV.W   #0, svMarkedBp
    MOV.W   #0, R12
    MOV.W   #0, R13
    CALLA   #__elk_backup
    MOV.W	#0, &P5IFG
    MOV.W	#0, &P4IFG
    NOP
    EINT
    NOP
    JMP     __scheduler_run
_verify_failed:
    JMP     __crt0_start    ;Just return to main and run as normal if no checkpoint

resetvec_ckpt:
.short	Port3_ON_handler
