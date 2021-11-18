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
; Sua matrícula e seu nome
;;;;
;-------------------------------------------------------------------------------
; Main loop here
;-------------------------------------------------------------------------------
PROVA_A0:	MOV		#SEQ1,R5 	;Endereço de início
			MOV		#8,R6 		;Quantidade de elementos
 			CALL	#CHECK_FIB
 			JMP $
 			NOP
;
; Sub-rotina
; Recebe: 	R5 = endereço de início da sequência a ser verificada
; 			R6 = quantidade de elementos da sequência
; Retorna: 	R7 = se OK (R7=1) se errada (R7=0)
; 			R8 = posição do primeiro elemento errado
;
CHECK_FIB: ;;;;coloque aqui sua sub-rotina
			;conferir se os dois primeiros numeros sao 0 e 1 - se for diferente, aciona a flag R7=0 e R8=posicao
			;conferir se o elemento atual é a soma dos dois anteriores
			MOV		#1,R7			;flag R7 começa em 1
			CMP 	#0,R5			;comparar o valor 0 com o primeiro item de R5
			;o que fazer se der diferente? Como fazer um if e colocar a posição de R5 em R8?
			INCD	R5				;passa R5 para o próximo valor do vetor
			CMP		#1,R5			;compara o segundo item de R5 com o valor 1
			;o que fazer se der diferente?
			INCD	R5				;aumentar R5 para ir para o próximo item
LOOP:		MOV		-4(R5),0(R9)	;mover o valor de R5 que está 2 posições na frente da atual para o R9
			ADD		-2(R5),0(R9)	;adicionar o valor de R5 que está 1 posição na frente da posição atual ao valor de R9
			COMP	R9,R5			;compara o valor de R9(soma) com o que foi fornecido para ver se está correto
			;se der diferente, tem que mudar o valor de R7 para 0, setar a posição em R8 e finalizar o loop

			;Duvidas: como indicar a posicao do vetor R5? Como ir para a próxima posição?
			;Como fazer um if e else?
			RET
; Diferentes sequências para provar seu programa
		.data
SEQ1:	.word	0,1,1,2,3,7,9,15 ;8 elementos e de o nr 6 é o primeiro errado

SEQ2:	.word 	1,0,1,2,3,5,8,13 ;8 elementos e de o nr 1 é o primeiro errado

SEQ3:	.word 	0,1,1,2,3,5,8,13,22,35,57 ;11 elementos e o nr 9 é o primeiro errado

SEQ4:	.word	0,1,1,2,3,5,8,13,21,34,55,89,144,233,377,610 ;25 elementos o de nr 22
		.word	987,1597,2584,4181,6765,10940,17711,28657,46368 ;é o primeiro errado

SEQ_OK:	.word	0,1,1,2,3,5,8,13,21,34,55,89,144,233,377,610 ;25 elementos
		.word	987,1597,2584,4181,6765,10946,17711,28657,46368 ;sem erros
                                            

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
            
