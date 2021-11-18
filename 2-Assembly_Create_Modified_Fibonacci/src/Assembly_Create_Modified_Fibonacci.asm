;-------------------------------------------------------------------------------
; MSP430 Assembler Code Template for use with TI Code Composer Studio
;
;
;-------------------------------------------------------------------------------
            .cdecls C,LIST,"msp430.h"       ; Include device header file
            
;-------------------------------------------------------------------------------
            .def    RESET                   ; Export program entry-point to
                                            ; make it known to linker.
;-------------------------------------------------------------------------------
            .text                           ; Assemble into program memory.
            .retain                         ; Override ELF conditional linking
                                            ; and retain current section.
            .retainrefs                     ; And retain any sections that have
                                            ; references to current section.

;-------------------------------------------------------------------------------
RESET       mov.w   #__STACK_END,SP         ; Initialize stackpointer
StopWDT     mov.w   #WDTPW|WDTHOLD,&WDTCTL  ; Stop watchdog timer


;-------------------------------------------------------------------------------
; Main loop here
;-------------------------------------------------------------------------------
			MOV		#SEQTRIB,R5
			CALL	#TRIB
			JMP		$
			NOP

TRIB:		MOV 	R5,R10		;mover o primeiro valor para R10
			INCD	R5			;avanço no vetor R5
			MOV		R5,R11		;mover o segundo valor para R11
			INCD	R5			;avanço no vetor R5
			MOV		R5,R12		;mover o terceiro valor para R12
			INCD	R5			;avanço no vetor R5
			MOV		#3,R6		;valor inicial de R6
LOOP:		MOV		@R12,R9		;mover o terceiro valor para R9
			ADD		@R11,R9		;adicionar A(n-1) + A(n-2)
			JC		END			;finalizar se tiver carry
			ADD		@R11,R9		;A(n-1) + A(n-2) + A(n-2)
			JC		END			;finalizar se tiver carry
			SUB		@R10,R9		;A(n-1) + A(n-2) + A(n-2) - A(n-3)
			MOV		R9,0(R5)	;mover resultado para o vetor R5
			INCD	R5			;avanço em R5
			INCD	R10			;avanço em R10
			INCD	R11			;avanço em R11
			INCD	R12			;avanço em R12
			INC		R6			;aumentar o valor de R6
			MOV		R9,R7		;mover maior valor para R7
			JNZ		LOOP
END:		RET


			.data
SEQTRIB:	.word	0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
			.word	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
			.word	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

                                            

;-------------------------------------------------------------------------------
; Stack Pointer definition
;-------------------------------------------------------------------------------
            .global __STACK_END
            .sect   .stack
            
;-------------------------------------------------------------------------------
; Interrupt Vectors
;-------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .short  RESET
            
