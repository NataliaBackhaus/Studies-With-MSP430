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
RT_TAM		.equ		32		;Tamanho dos rotores (32 letras)

VISTO1:		MOV			#MSG,R5		;Mover o endereço de memória de MSG para o registrador 5
			MOV			#GSM,R6		;Mover o endereço de memória de GSM para o registrador 6
			MOV			#RT_TAM,R8	;Mover o tamanho do rotor para o registrador 8
			MOV			#CHAVE,R4	;Mover o vetor CHAVE para R4
			MOV.B		1(R4),R12	;Mover a configuração do primeiro rotor para o registrador 12
			CALL		#ENIGMA		;Cifrar
			CALL		#RT_RESET	;Voltar à posição inicial
			MOV			#GSM,R5		;Mover o endereço de memória de GSM para o registrador 5
			MOV			#DCF,R6		;Mover o endereço de memória de DCF para o registrador 6
			CALL		#ENIGMA		;Decifrar
			JMP			$
			NOP
;
; Definição da chave do Enigma
CHAVE:	.byte	2, 6, 3, 10, 5, 12, 2

;
; Sua Rotina Enigma
ENIGMA:		TST.B	0(R5)
			JNZ		SEG				;Repetir se diferente de zero
			RET
SEG:		MOV.B	@R5+,R7			;Armazena um elemento da mensagem em R7
			CMP.B	#';',R7			;Compara se o caracter é menor que ';' na tabela ASCII
			JL		CONSERVA		;Se for menor, ativa a subrotina CONSERVA para conservar o caracter
			CMP.B	#'[',R7			;Compara se o caracter é maior ou igual que '[' na tabela ASCII
			JGE		CONSERVA		;Se for maior ou igual, ativa a subrotina CONSERVA para conservar o caracter
			SUB.B	#';',R7			;Subtrai ';' de R7 para descobrir o valor entre 0 e RT_TAM
			ADD.B	R12,R7			;Aplicando a configuração
			CMP.B	R8,R7			;Verificar se o valor de R7 é maior que 'Z'
			JGE		CONFIGURA		;Ativar subrotina para configurar de forma correta se for maior que 'Z'
SEG2:		MOV.B	RT2(R7),R7		;Aplicando o rotor 2 (hard coded)
			MOV.B	RF2(R7),R7		;Aplicando o refletor 2 (hard coded)
			MOV		#RT2,R10		;Movendo o rotor 2 para R10
			CALL	#C_INV			;Chamar subrotina C_INV
			ADD.B	#';',R7			;Adiciona ';' em R7
			SUB.B	R12,R7			;Subtrai a configuração (R12) de R7
			CMP.B	#';',R7			;Compara R7 com ';'
			JHS		ARMAZENA		;Se for maior ou igual, vai para a rotina ARMAZENA
			ADD.B	R8,R7			;Adiciona R8 e R7
ARMAZENA:	MOV.B	R7,0(R6)		;Move R7 para a primeira posição de R6
			INC		R6				;Incrementa R6
			JMP		ENIGMA			;Volta para ENIGMA
RT_RESET:	CLR		R5				;Limpar R5
			CLR		R6				;Limpar R6
			CLR		R7				;Limpar R7
			CLR		R10				;Limpar R10
			CLR		R11				;Limpar R11
CONSERVA:	MOV.B	R7,0(R6)		;Move R7 para a primeira posição de R6
			INC		R6				;Incrementa R6
			JMP		ENIGMA			;Volta para ENIGMA
CONFIGURA:	SUB.B	R8,R7			;Subtrai R8 de R7
			JMP		SEG2			;Volta para a rotina SEG2

;
; Consulta inversa
; Recebe:	R10=endereço do rotor
;			R7 =elemento a ser buscado
; Retorna:	R7 =índice do elemento
; Usa:		R11=contador (0, 1, ...)
C_INV:		CLR			R11			;Limpar R11
CI0:		CMP.B		@R10+,R7	;Comparar o valor de R7 com o elemento do rotor
			JNZ			CI1			;Se for diferente de zero, chama CI1
			MOV.B		R11,R7		;Caso contrário, armazena o contador R11 em R7
			RET
CI1:		INC			R11			;Incrementa o contador R11
			JMP			CI0			;Retorna para CI0
			NOP

;
; Área de dados
		.data
MSG:	.byte	"UMA NOITE DESTAS, VINDO DA CIDADE PARA O ENGENHO NOVO,"
		.byte	" ENCONTREI NO TREM DA CENTRAL UM RAPAZ AQUI DO BAIRRO,"
		.byte 	" QUE EU CONHECO DE VISTA E DE CHAPEU.",0

GSM:	.byte	"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
		.byte	"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
		.byte 	"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",0

DCF:	.byte	"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
		.byte	"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
		.byte 	"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",0

;Rotores com 32 posições
ROTORES:
RT1:	.byte	13, 23,  0,  9,  4,  2,  5, 11, 12, 17, 21,  6, 28, 25, 30, 10
		.byte	22,  1,  3, 26, 24, 31,  8, 14, 29, 15, 18, 16, 19,  7, 27, 20
RT2:	.byte	 6, 24,  2,  8, 25, 20, 16, 29, 23,  0,  7, 19, 30, 17, 12, 15
		.byte	 5,  4, 26, 10, 11, 18, 28, 27, 14,  9, 13,  1, 21, 31, 22,  3
RT3:	.byte	 6, 15, 23,  7, 27, 13, 19,  3, 16,  4, 17, 20, 24, 25,  0, 10
		.byte	30, 26, 22,  1,  8, 11, 14, 31,  9, 28,  5, 18, 12,  2, 29, 21
RT4:	.byte	15, 16,  5, 18, 31, 26, 19, 28,  1,  2, 14, 12, 24, 20, 21,  0
		.byte	11, 23,  4, 10,  7,  3, 25, 29, 27,  8, 17,  6,  9, 13, 22, 30
RT5:	.byte	13, 25,  1, 26,  6, 12,  9,  2, 28, 11, 16, 15,  4,  8,  3, 31
		.byte	 5, 18, 23, 17, 24, 27,  0, 22, 29, 19,  7, 10, 14, 21, 20, 30

;Refletores com 32 posições
REFLETORES:
RF1:	.byte	26, 23, 31,  9, 29, 20, 16, 11, 27,  3, 14,  7, 21, 28, 10, 25
		.byte	 6, 22, 24, 30,  5, 12, 17,  1, 18, 15,  0,  8, 13,  4, 19,  2
RF2:	.byte	20, 29,  8,  9, 23, 27, 21, 11,  2,  3, 25,  7, 13, 12, 22, 16
		.byte	15, 28, 30, 26,  0,  6, 14,  4, 31, 10, 19,  5, 17,  1, 18, 24
RF3:	.byte	14, 30,  7,  5, 15,  3, 18,  2, 23, 17, 29, 28, 25, 27,  0,  4
		.byte	19,  9,  6, 16, 26, 22, 21,  8, 31, 12, 20, 13, 11, 10,  1, 24

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
            
