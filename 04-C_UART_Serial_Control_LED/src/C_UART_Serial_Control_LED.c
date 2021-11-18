#include <msp430.h> 

#define TRUE  1
#define FALSE 0

void bt_str(char *vet);
void bt_char(char c);
void led_VM(void);
void led_vm(void);
void led_vm_inv(void);
void led_VD(void);
void led_vd(void);
void led_vd_inv(void);
void leds_config(void);
void USCI_A1_config(void);

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	__delay_cycles(500000); //Atraso de 0,5 seg para garantir estabilidade
	USCI_A1_config();
	leds_config();

	//printar comandos iniciais
	bt_str("Prova A4.\n");
	bt_str("Comandos: VM, vm, Vm, vM, VD, vd, Vd, vD.\n");
	bt_str("Pronto!\n");

	//logica do circuito
	char i;
	volatile char cmdo[20];
	for (i=0; i<20; i++)    cmdo[i]=0;   //Zerar vetor
	i=0;
	while(i<20){
	    whille( (UCA1IFG&UCRXIFG) == 0);
	    cmdo[i++] = UCA1RXBUF;
	    int a = 0;
	    while(a<19){
	        //possibilidades com o led vermelho
	        if (cmdo[a] == 'V'){
	            if (cmdo[a+1] == 'M'){
	                led_VM();
	                bt_str("Vermelho aceso.\n");
	            }
	        }
	        if (cmdo[a] == 'v'){
	            if (cmdo[a+1] == 'm'){
	                led_vm();
	                bt_str("Vermelho apagado.\n");
	            }
	        }
	        if (cmdo[a] == 'V'){
	            if (cmdo[a+1] == 'm'){
	                led_vm_inv();
	                bt_str("Vermelho invertido.\n");
	            }
	        }
	        if (cmdo[a] == 'v'){
	            if (cmdo[a+1] == 'M'){
	                led_vm_inv();
	                bt_str("Vermelho invertido.\n");
	            }
	        }
	        //possibilidades com o led verde
	        if (cmdo[a] == 'V'){
	            if (cmdo[a+1] == 'D'){
	                led_VD();
	                bt_str("Verde aceso.\n");
	            }
	        }
	        if (cmdo[a] == 'v'){
	            if (cmdo[a+1] == 'd'){
	                led_vd();
	                bt_str("Verde apagado.\n");
	            }
	        }
	        if (cmdo[a] == 'V'){
	            if (cmdo[a+1] == 'd'){
	                led_vm_inv();
	                bt_str("Verde invertido.\n");
	            }
	        }
	        if (cmdo[a] == 'v'){
	            if (cmdo[a+1] == 'D'){
	                led_vm();
	                bt_str("Verde invertido.\n");
	            }
	        }
	        a = a+1;
	    }

	}
	while(1);
	return 0;
}

//Enviar uma string pela serial
void bt_str(char *vet){
    int i=0;
    while (vet[i] != 0){
        bt_char(vet[i]);
        i++;
    }
}

//Enviar um caracter pela serial
void bt_char(char c){
    UCA1TXBUF=c;
    while((UCA1IFG&UCTXIFG)==0);    //Esperar TXIFG=1
}

//Controle dos leds
void led_VM(void){ P1OUT |=  BIT0; }     //led Vermelho aceso
void led_vm(void){ P1OUT &= ~BIT0; }     //led Vermelho aceso
void led_vm_inv(void){ P1OUT ^= BIT0; }  //inverter led Vermelho
void led_VD(void){ P4OUT |=  BIT7; }     //led Verde aceso
void led_vd(void){ P4OUT &= ~BIT7; }     //led Verde aceso
void led_vd_inv(void){ P4OUT ^= BIT7; }  //inverter led Verde

//Configurar leds
void leds_config(void){
    P1DIR |= BIT0;      P1OUT &= ~BIT0;  //led vermelho
    P4DIR |= BIT7;      P4OUT &= ~BIT7;  //led verde
}

//Configurar USCI_A1 em 38.400
void USCI_A1_config(void){
    UCA1CTL1 = UCSWRST | UCSSEL_2;              //RST = 1 e SMCLK
    UCA1CTL0 = 0;                               //sem paridade, 8 bits, 1 stop, modo UART
    UCA1BRW = 27;                               //Divisor
    UCA1MCTL = UCBRF_0 | UCBRS_2 & ~UCOS16;     //Moduladores 2, UCOS16=0
    P4SEL |= BIT5|BIT4;
    UCA1CTL1 &= ~UCSWRST;                       //RST = 0
}
