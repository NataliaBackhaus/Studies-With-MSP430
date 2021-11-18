// Básico para operar com LCD
// P3.0 = SDA
// P3.1 = SCL

#include <msp430.h> 

#define ADC12MEM2ADDR   (__SFR_FARPTR) 0x0724

#define TRUE    1
#define FALSE   0

// Definição do endereço do PCF_8574
#define PCF_ADR1 0x3F
#define PCF_ADR2 0x27
#define PCF_ADR  PCF_ADR2

#define BR_100K    11  //SMCLK/100K = 11
#define BR_50K     21  //SMCLK/50K  = 21
#define BR_10K    105  //SMCLK/10K  = 105

// Criar caracteres especiais
char col_0[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F};
char col_1[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F};
char col_2[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F};
char col_3[8] = {0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F};
char col_4[8] = {0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};
char col_5[8] = {0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};
char col_6[8] = {0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};
char col_7[8] = {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};

// Protótipo das principais funções
void uart1_config(void);
void ta0_config(void);
void adc_config(void);
void dma1_config(void);
void ser_str(char *pt);
void ser_char(char x);
int exponencial2(int expoente);
void lcd_char(char x);
void lcd_esp(char x, char *vt);
void lcd_cursor(char x);
void lcd_cmdo(char x);
void lcd_inic(void);
void lcd_aux(char dado);
int pcf_read(void);
void pcf_write(char dado);
int pcf_teste(char adr);
void i2c_config(void);
void delay(long limite);
void clear_display(void);

volatile int adc_vet[4];    //Receber o resultado do ADC
volatile int freq[16];      //Contar a frequência de cada número
char maskLsb = 0x01;        //Máscara para pegar apenas o LSB
int count_adc = 0;          //Contador do ADC
int percent_vec[16];        //Armazena as porcentagens
int main(void){
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    __delay_cycles(500000);     //Atraso de 0,5 seg para garantir estabilidade
    uart1_config();
    ta0_config();
    adc_config();
    dma1_config();
    i2c_config();
    if (pcf_teste(PCF_ADR)==FALSE){
        while(TRUE);        //Travar
    }

    lcd_inic();     //Inicializar LCD
    pcf_write(8);   //Acender Back Light
    ser_str("Prova B2q2\n");
    ser_str("  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15\n");

    // Caracter
    lcd_esp(0,col_0);
    lcd_esp(1,col_1);
    lcd_esp(2,col_2);
    lcd_esp(3,col_3);
    lcd_esp(4,col_4);
    lcd_esp(5,col_5);
    lcd_esp(6,col_6);
    lcd_esp(7,col_7);

    __enable_interrupt();


    while(TRUE){
        ;
    }
    return 0;
}
//#pragma vector = 54
#pragma vector = ADC12_VECTOR
__interrupt void adc_int(void){
    int i = 0;                          //Contador do for
    int num_aleatorio = 0;              //Valor do número aleatório gerado
    int valor = 0;                      //LSB do ADC
    char mx[4]={};                      //Vetor auxiliar para printar a porcentagem
    DMA1CTL |= DMAREQ;                  //Iniciar DMA
    while ( (DMA1CTL&DMAIFG) == 0);     //Espersar DMAIFG
    for(i=0; i<4; i++){
        valor = adc_vet[i]&maskLsb;                                 //Aplicar a máscara para pegar o LSB
        num_aleatorio = valor*exponencial2(i) + num_aleatorio;      //Calcula o número aleatório
    }
    percent_vec[num_aleatorio]++;       //Quantas vezes o número aleatório aconteceu
    count_adc++;                        //Incrementar contador do ADC
    if(count_adc == 1000){
        clear_display();                //Função criada para limpar a tela do LCD
        for(i = 0; i < 16; i++){        //Loop para calcular e printar o valor da porcentagem de cada número aleatório
            percent_vec[i] = (percent_vec[i]*100)/1000;     //Calcula a porcentagem de cada número
            mx[0] = '0' + ((percent_vec[i]/100)%10);        // hundreds digit
            mx[1] = '0' + ((percent_vec[i]/10)%10);         // tens digit
            mx[2] = '0' + (percent_vec[i]%10);              // ones digit
            ser_str(mx);
            ser_str(" ");

            //Definição dos possíveis valores do histograma
            switch (percent_vec[i]){
                case 0: //0%
                    lcd_cursor(i+64);
                    lcd_char(0);
                    break;
                case 1: //1%
                    lcd_cursor(i+64);
                    lcd_char(1);
                    break;
                case 2: //2%
                    lcd_cursor(i+64);
                    lcd_char(2);
                    break;
                case 3: //3%
                    lcd_cursor(i+64);
                    lcd_char(3);
                    break;
                case 4: //4%
                    lcd_cursor(i+64);
                    lcd_char(4);
                    break;
                case 5: //5%
                    lcd_cursor(i+64);
                    lcd_char(5);
                    break;
                case 6: //6%
                    lcd_cursor(i+64);
                    lcd_char(6);
                    break;
                case 7: //7%
                    lcd_cursor(i+64);
                    lcd_char(7);
                    break;
                case 8: //8%
                    lcd_cursor(i+64);
                    lcd_char(7);
                    lcd_cursor(i);
                    lcd_char(0);
                    break;
                case 9: //9%
                    lcd_cursor(i+64);
                    lcd_char(7);
                    lcd_cursor(i);
                    lcd_char(1);
                    break;
                case 10: //10%
                    lcd_cursor(i+64);
                    lcd_char(7);
                    lcd_cursor(i);
                    lcd_char(2);
                    break;
                case 11: //11%
                    lcd_cursor(i+64);
                    lcd_char(7);
                    lcd_cursor(i);
                    lcd_char(3);
                    break;
                case 12: //12%
                    lcd_cursor(i+64);
                    lcd_char(7);
                    lcd_cursor(i);
                    lcd_char(4);
                    break;
                case 13: //13%
                    lcd_cursor(i+64);
                    lcd_char(7);
                    lcd_cursor(i);
                    lcd_char(5);
                    break;
                case 14: //14%
                    lcd_cursor(i+64);
                    lcd_char(7);
                    lcd_cursor(i);
                    lcd_char(6);
                    break;
                default: //Acima de 14%
                    lcd_cursor(i+64);
                    lcd_char(7);
                    lcd_cursor(i);
                    lcd_char(7);
                    break;

            }


            percent_vec[i] = 0;     //Zerar a porcentagem

        }
        ser_str("\n");
        count_adc = 0;              //Zerar o contador ADC

    }
}

// Funções para inicializar os recursos
void uart1_config(void){
    UCA1CTL1 = UCSSEL_2 | UCSWRST;              //RST = 1 e SMCLK
    UCA1CTL0 = 0;                               //sem paridade, 8 bits, 1 stop, modo UART
    UCA1BRW = 18;                               //Divisor
    UCA1MCTL = UCBRF_0 | UCBRS_1 & ~UCOS16;     //Moduladores 2, UCOS16=0
    P4SEL |= BIT5|BIT4;
    UCA1CTL1 &= ~UCSWRST;                       //RST=0
}

void ta0_config(void){
    TA0CTL = TASSEL_2 | MC_1;
    TA0CCTL1 = OUTMOD_6;                        //Out = modo 6
    TA0CCR0 = 1048576/66000;                    //66 kHz
    TA0CCR1 = TA0CCR0/2;                        //Carga 50%
    P1DIR |= BIT2;                              //Sair TA0.1
    P1SEL |= BIT2;
}

//Configurar ADC
void adc_config(void){
    volatile unsigned char *pt;
    unsigned char i;
    ADC12CTL0 &= ~ADC12ENC;                    //Desabilitar para configurar
    ADC12CTL0 = ADC12ON;                       //Ligar ADC
    ADC12CTL1 = ADC12CONSEQ_3 |                //Modo sequência de canais repetido
    ADC12SHS_1 |                               //Selecionar TA0.1
    ADC12CSTARTADD_2 |                         //Resultado a partir de ADC12MEM2
    ADC12SSEL_3;                               //ADC12CLK = SMCLK
    ADC12CTL2 = ADC12RES_2;                    //Modo 12 bits
    pt=&ADC12MCTL2;

    for (i=0; i<4; i++)
     pt[i]=ADC12SREF_0 | ADC12INCH_1;          //ADC12MCTL2 até ADC12MCTL5
    pt[3] |= ADC12EOS;                         //EOS em ADC12MCTL5
    P6SEL |= BIT1|BIT0;                        // Desligar digital de P6.0,1
    ADC12CTL0 |= ADC12ENC;                     //Habilitar ADC12
    ADC12IE |= ADC12IE5;                       //Hab interrupção MEM5
}

// Configurar DMA1
void dma1_config(void){
    DMACTL0 = DMA1TSEL_0;                      //DREQ
    DMA1CTL = DMADT_5 |
              DMADSTINCR_3 |
              DMASRCINCR_3;
    DMA1SA = ADC12MEM2ADDR;                    //Endereço fonte
    DMA1DA = adc_vet;                          //Endereço destino
    DMA1SZ = 4;                                //Quantidade
    DMA1CTL |= DMAEN;                          //Habilitar
}

//Função para calcular a exponencial de 2^expoente
int exponencial2(int expoente) {
    int i = 1;
    int resultado = 1;
    for(i = 1; i <= expoente; i++){
        resultado = resultado*2;
    }
    return resultado;
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

// Imprimir uma letra no LCD (x = abcd efgh)
void lcd_char(char x){
    char lsn,msn;   //nibbles
    lsn=(x<<4)&0xF0;        //lsn efgh 0000
    msn=x&0xF0;             //msn abcd 0000
    pcf_write(msn | 0x9);
    pcf_write(msn | 0xD);
    pcf_write(msn | 0x9);
    ;
    pcf_write(lsn | 0x9);
    pcf_write(lsn | 0xD);
    pcf_write(lsn | 0x9);
}

// Posicionar cursor
void lcd_cursor(char x){
    lcd_cmdo(0x80 | x);
}

// Enviar um comando (RS=0) para o LCD (x = abcd efgh)
void lcd_cmdo(char x){
    char lsn,msn;   //nibbles
    lsn=(x<<4)&0xF0;        //lsn efgh 0000
    msn=x&0xF0;             //msn abcd 0000
    pcf_write(msn | 0x8);
    pcf_write(msn | 0xC);
    pcf_write(msn | 0x8);
    ;
    pcf_write(lsn | 0x8);
    pcf_write(lsn | 0xC);
    pcf_write(lsn | 0x8);
}

// Mapeia no caracter "x" o vetor vt[]
// Reposiciona o cursor em lin=0 e col=0
void lcd_esp(char x, char *vt){
    unsigned int adr,i;
    adr = x<<3;
    lcd_cmdo(0x40 | adr);
    for (i=0; i<8; i++)
        lcd_char(vt[i]);
    lcd_cursor(0);
}

// Incializar LCD modo 4 bits
void lcd_inic(void){

    // Preparar I2C para operar
    UCB0I2CSA = PCF_ADR;    //Endereço Escravo
    UCB0CTL1 |= UCTR    |   //Mestre TX
                UCTXSTT;    //Gerar START
    while ( (UCB0IFG & UCTXIFG) == 0);          //Esperar TXIFG=1
    UCB0TXBUF = 0;                              //Saída PCF = 0;
    while ( (UCB0CTL1 & UCTXSTT) == UCTXSTT);   //Esperar STT=0
    if ( (UCB0IFG & UCNACKIFG) == UCNACKIFG)    //NACK?
                while(1);

    // Começar inicialização
    lcd_aux(0);     //RS=RW=0, BL=1
    delay(20000);
    lcd_aux(3);     //3
    delay(10000);
    lcd_aux(3);     //3
    delay(10000);
    lcd_aux(3);     //3
    delay(10000);
    lcd_aux(2);     //2

    // Entrou em modo 4 bits
    lcd_aux(2);     lcd_aux(8);     //0x28
    lcd_aux(0);     lcd_aux(8);     //0x08
    lcd_aux(0);     lcd_aux(1);     //0x01
    lcd_aux(0);     lcd_aux(6);     //0x06
    lcd_aux(0);     lcd_aux(0xF);   //0x0F

    while ( (UCB0IFG & UCTXIFG) == 0)   ;          //Esperar TXIFG=1
    UCB0CTL1 |= UCTXSTP;                           //Gerar STOP
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP)   ;   //Esperar STOP
    delay(50);
}

// Auxiliar inicialização do LCD (RS=RW=0)
// *** Só serve para a inicialização ***
void lcd_aux(char dado){
    while ( (UCB0IFG & UCTXIFG) == 0);              //Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT3;            //PCF7:4 = dado;
    delay(50);
    while ( (UCB0IFG & UCTXIFG) == 0);              //Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT3 | BIT2;     //E=1
    delay(50);
    while ( (UCB0IFG & UCTXIFG) == 0);              //Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT3;            //E=0;
}

// Ler a porta do PCF
int pcf_read(void){
    int dado;
    UCB0I2CSA = PCF_ADR;                //Endereço Escravo
    UCB0CTL1 &= ~UCTR;                  //Mestre RX
    UCB0CTL1 |= UCTXSTT;                //Gerar START
    while ( (UCB0CTL1 & UCTXSTT) == UCTXSTT);
    UCB0CTL1 |= UCTXSTP;                //Gerar STOP + NACK
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP)   ;   //Esperar STOP
    while ( (UCB0IFG & UCRXIFG) == 0);  //Esperar RX
    dado=UCB0RXBUF;
    return dado;
}

// Escrever dado na porta
void pcf_write(char dado){
    UCB0I2CSA = PCF_ADR;        //Endereço Escravo
    UCB0CTL1 |= UCTR    |       //Mestre TX
                UCTXSTT;        //Gerar START
    while ( (UCB0IFG & UCTXIFG) == 0)   ;          //Esperar TXIFG=1
    UCB0TXBUF = dado;                              //Escrever dado
    while ( (UCB0CTL1 & UCTXSTT) == UCTXSTT)   ;   //Esperar STT=0
    if ( (UCB0IFG & UCNACKIFG) == UCNACKIFG)       //NACK?
                while(1);                          //Escravo gerou NACK
    UCB0CTL1 |= UCTXSTP;                        //Gerar STOP
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP)   ;   //Esperar STOP
}

// Testar endereço I2C
// TRUE se recebeu ACK
int pcf_teste(char adr){
    UCB0I2CSA = adr;                            //Endereço do PCF
    UCB0CTL1 |= UCTR | UCTXSTT;                 //Gerar START, Mestre transmissor
    while ( (UCB0IFG & UCTXIFG) == 0);          //Esperar pelo START
    UCB0CTL1 |= UCTXSTP;                        //Gerar STOP
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP);   //Esperar pelo STOP
    if ((UCB0IFG & UCNACKIFG) == 0)     return TRUE;
    else                                return FALSE;
}

// Configurar UCSB0 e Pinos I2C
// P3.0 = SDA e P3.1=SCL
void i2c_config(void){
    UCB0CTL1 |= UCSWRST;    // UCSI B0 em ressete
    UCB0CTL0 = UCSYNC |     //Síncrono
               UCMODE_3 |   //Modo I2C
               UCMST;       //Mestre
    UCB0BRW = BR_100K;      //100 kbps
    P3SEL |=  BIT1 | BIT0;  // Use dedicated module
    UCB0CTL1 = UCSSEL_2;    //SMCLK e remove ressete
}

void delay(long limite){
    volatile long cont=0;
    while (cont++ < limite) ;
}

void clear_display(void){
    lcd_cursor(0x00);
    int i = 0;
    for(i = 0; i < 32; i++){
        lcd_char(' ');
    }
}
