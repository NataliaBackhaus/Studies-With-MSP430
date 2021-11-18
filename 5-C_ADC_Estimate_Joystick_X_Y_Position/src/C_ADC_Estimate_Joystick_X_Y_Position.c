#include <msp430.h> 

#define FALSE 0
#define TRUE 1

// Funções
void ser_str(char *vet);
void ser_char(char c);
void uart1_config(void);
void adc_config(void);
void tb0_config(void);
void GPIO_config(void);

// Variáveis globais
volatile int media_x, media_y;

int main(void){
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    __delay_cycles(500000);     //Atraso de 0,5 seg para garantir estabilidade
    uart1_config();
    //GPIO_config();
    tb0_config();
    adc_config();
    __enable_interrupt();

    //printar comandos iniciais
    ser_str("Prova A5\n");

    while(TRUE){
    }
    return 0;
}

//#pragma vector = 54
#pragma vector = ADC12_VECTOR
__interrupt void adc_int(void){
     volatile unsigned int *pt;
     unsigned int i,soma, pos_x;
     char mx[4] = {0};
     char my[4] = {0};
     char display[64] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','|','|',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','|'};
     pt = &ADC12MEM7;
     soma = 0;
     for (i=0; i<4; i++) soma +=pt[i];
     media_x = soma/4;
     pos_x = media_x/8;
     display[pos_x] = '*';
     soma = 0;
     for (i=4; i<7; i++) soma +=pt[i];
     media_y = soma/3;

     mx[0] = '0' + ((media_x/100)%10);  // hundreds digit
     mx[1] = '0' + ((media_x/10)%10);   // tens digit
     mx[2] = '0' + (media_x%10);        // ones digit

     my[0] = '0' + ((media_y/100)%10);  // hundreds digit
     my[1] = '0' + ((media_y/10)%10);   // tens digit
     my[2] = '0' + (media_y%10);        // ones digit

     ser_str(my);
     ser_str(" ");
     ser_str(mx);
     ser_str("  |");
     ser_str(display);
     ser_str("\n");
}

//Enviar uma string pela serial
void ser_str(char *pt){
    int i=0;
    while (pt[i]!=0){
        ser_char(pt[i]);
        i++;
    }
}

//Enviar um caracter pela serial
void ser_char(char x){
    UCA1TXBUF=x;
    while ((UCA1IFG&UCTXIFG)==0);   //Esperar transmitir
}

//Configurar USCI_A1 em 57.600
void uart1_config(void){
    UCA1CTL1 = UCSSEL_2 | UCSWRST;              //RST = 1 e SMCLK
    UCA1CTL0 = 0;                               //sem paridade, 8 bits, 1 stop, modo UART
    UCA1BRW = 18;                               //Divisor
    UCA1MCTL = UCBRF_0 | UCBRS_1 & ~UCOS16;     //Moduladores 2, UCOS16=0
    P4SEL |= BIT5|BIT4;
    UCA1CTL1 &= ~UCSWRST;                       //RST=0
}

//Configurar ADC
void adc_config(void){
     volatile unsigned char *pt;
     unsigned char i;
     ADC12CTL0 &= ~ADC12ENC;                    //Desabilitar para configurar
     ADC12CTL0 = ADC12ON;                       //Ligar ADC
     ADC12CTL1 = ADC12CONSEQ_3 |                //Modo sequência de canais repetido
     ADC12SHS_3 |                               //Selecionar TB0.1
     ADC12CSTARTADD_7 |                         //Resultado a partir de ADC12MEM0
     ADC12SSEL_3;                               //ADC12CLK = SMCLK
     ADC12CTL2 = 0;                             //ADC12RES=0, Modo 8 bits
     pt=&ADC12MCTL7;

     for (i=0; i<4; i++)
         pt[i]=ADC12SREF_0 | ADC12INCH_0;       //ADC12MCTL0 até ADC12MCTL7
     for (i=4; i<7; i++)
         pt[i]=ADC12SREF_0 | ADC12INCH_1;       //ADC12MCTL8 até ADC12MCTL15
     pt[6] |= ADC12EOS;                         //EOS em ADC12MCTL15
     P6SEL |= BIT1|BIT0;                        // Desligar digital de P6.0,1
     ADC12CTL0 |= ADC12ENC;                     //Habilitar ADC12
     ADC12IE |= ADC12IE13;                      //Hab interrupção MEM6
}

// Configurar o timer TB0.1
void tb0_config(void){
    TB0CTL = TBSSEL_1 | MC_1;
    TB0CCTL1 = OUTMOD_6;        //Out = modo 6
    TB0CCR0 = 32767/91;         //91 Hz (1/0.011=91Hz)
    TB0CCR1 = TB0CCR0/2;        //Carga 50%
    P4DIR |= BIT0;              //P4.0 como saída
    P4SEL |= BIT0;              //P4.0 saída alternativa
    PMAPKEYID = 0X02D52;        //Liberar mapeamento
    P4MAP0 = PM_TB0CCR1A;       //TB0.1 saí por P4.0
}

/*void GPIO_config(void){
    P3DIR &= ~BIT4;             //P3.4 = SW
    P3REN |= BIT4;
    P3OUT |= BIT4;
}
*/
