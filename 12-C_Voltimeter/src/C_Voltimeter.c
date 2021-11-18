#include <msp430.h> 

#define FALSE 0
#define TRUE 1
#define VRX 0 //Indicar canal VRx
#define VRY 1 //Indicar canal VRy
#define FECHADA 0 //SW fechada
#define ABERTA 1 //SW aberta
#define DBC 1000 //Debounce

// Funções
void adc_config(void);
void tb0_config(void);
void GPIO_config(void);
int sw_mon(void);
void led_VM(void);
void led_vm(void);
void debounce(int valor);
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
void print_modo(int modo_t, int tensao);
void buffer_func(int valor);
void reset_buffer(void);
int max_buffer(void);
int min_buffer(void);
int converter_adc_volt(int adc_value);
void print_modo3(int modo_t, int tensao_x, int tensao_y);

// Definição do endereço do PCF_8574
#define PCF_ADR1 0x3F
#define PCF_ADR2 0x27
#define PCF_ADR  PCF_ADR2

#define BR_100K    11  //SMCLK/100K = 11
#define BR_50K     21  //SMCLK/50K  = 21
#define BR_10K    105  //SMCLK/10K  = 105

// Variáveis globais
int media_x, media_y;
volatile int flag = 0;
volatile int modo = 0;

// Buffer Circular
#define BUFFER_SIZE 10
int buffer[BUFFER_SIZE]={};
int indexBuffer = 0;
int flag_buffer = 0;
int flag_change = 0;

int main(void){

    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    tb0_config();
    adc_config();
    GPIO_config();
    i2c_config();
    if (pcf_teste(PCF_ADR)==FALSE){
        led_VM();           //Indicar que não houve ACK
        while(TRUE);        //Travar
    }

    lcd_inic();     //Inicializar LCD
    pcf_write(8);   //Acender Back Light
    __enable_interrupt();

    // Definir o modo
    while(TRUE){
        if (sw_mon() == TRUE){
            modo++;
            flag_change = 1;
            if (modo == 3) modo = 0;
        }
        if (flag == 1){
            if(flag_change == 1) reset_buffer();
            switch (modo) {
                case 0:
                    buffer_func(media_x);
                    print_modo(modo, media_x);
                    break;
                case 1:
                    buffer_func(media_y);
                    print_modo(modo, media_y);
                    break;
                case 2:
                    buffer_func(media_x);
                    print_modo3(modo, media_x, media_y);
                   break;
                default:
                   ;
            }
            flag = 0;
        }
    }
    return 0;
}

//#pragma vector = 54
#pragma vector = ADC12_VECTOR
__interrupt void adc_int(void){
    volatile unsigned int *pt;
    unsigned int i,soma;
    pt = &ADC12MEM0;
    soma = 0;
    for (i=0; i<8; i++) soma +=pt[i];
    media_x = soma/8;
    soma = 0;
    for (i=8; i<16; i++) soma +=pt[i];
    media_y = soma/8;
    flag = 1;
    P1OUT ^= BIT0;
}

//Configurar ADC
void adc_config(void){
     volatile unsigned char *pt;
     unsigned char i;
     ADC12CTL0 &= ~ADC12ENC;                    //Desabilitar para configurar
     ADC12CTL0 = ADC12ON;                       //Ligar ADC
     ADC12CTL1 = ADC12CONSEQ_3 |                //Modo sequência de canais repetido
     ADC12SHS_3 |                               //Selecionar TB0.1
     ADC12CSTARTADD_0 |                         //Resultado a partir de ADC12MEM0
     ADC12SSEL_3;                               //ADC12CLK = SMCLK
     ADC12CTL2 = ADC12RES_2;                    //ADC12RES=0, Modo 8 bits
     pt=&ADC12MCTL0;

     for (i=0; i<8; i++)
         pt[i]=ADC12SREF_0 | ADC12INCH_0;       //ADC12MCTL0 até ADC12MCTL7
     for (i=8; i<16; i++)
         pt[i]=ADC12SREF_0 | ADC12INCH_1;       //ADC12MCTL8 até ADC12MCTL15
     pt[15] |= ADC12EOS;                        //EOS em ADC12MCTL15
     P6SEL |= BIT1|BIT0;                        // Desligar digital de P6.0,1
     ADC12CTL0 |= ADC12ENC;                     //Habilitar ADC12
     ADC12IE |= ADC12IE15;                      //Hab interrupção MEM6
}

// Configurar o timer TB0.1
void tb0_config(void){
    TB0CTL = TBSSEL_1 | MC_1;
    TB0CCTL1 = OUTMOD_6;        //Out = modo 6
    TB0CCR0 = 32767/64;         //64 Hz
    TB0CCR1 = TB0CCR0/2;        //Carga 50%
    P4DIR |= BIT0;              //P4.0 como saída
    P4SEL |= BIT0;              //P4.0 saída alternativa
    PMAPKEYID = 0X02D52;        //Liberar mapeamento
    P4MAP0 = PM_TB0CCR1A;       //TB0.1 saí por P4.0
}

// Configurar GPIO
void GPIO_config(void){
    P1DIR |= BIT0; P1OUT &= ~BIT0;  //Led Vermelho
    P6DIR &= ~BIT2;                 //P6.2 = SW
    P6REN |= BIT2;
    P6OUT |= BIT2;
}

// Configurar funções do LED
void led_VM(void){ P1OUT |= BIT0; }     //Acender Verm
void led_vm(void){ P1OUT &= ~BIT0; }    //Apagar Verm

// Debounce
void debounce(int valor){
    volatile int x;             //volatile evita optimizador
    for (x=0; x<valor; x++);    //Apenas gasta tempo
}

// Monitorar SW (P3.4), retorna TRUE se foi acionada
int sw_mon(void){
    static int psw=ABERTA;      //Guardar passado de Sw
    if ( (P6IN&BIT2) == 0){     //Qual estado atual de Sw?
        if (psw==ABERTA){       //Qual o passado de Sw?
            debounce(DBC);
            psw=FECHADA;
            return TRUE;
        }
    }
    else{
        if (psw==FECHADA){      //Qual o passado de Sw?
            debounce(DBC);
            psw=ABERTA;
            return FALSE;
        }
    }
    return FALSE;
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

// Função para printar o modo 1 e o modo 2
void print_modo(int modo_t, int tensao){
    char modo_char = modo_t+'0';
    int tensao_v = converter_adc_volt(tensao);
    int maxb = max_buffer();
    int max = converter_adc_volt(maxb);
    int minb = min_buffer();
    int min = converter_adc_volt(minb);

    char tensao_char[4] = {0};
    tensao_char[0] = '0' + ((tensao_v/10000)%10);
    tensao_char[1] = '0' + ((tensao_v/1000)%10);
    tensao_char[2] = '0' + ((tensao_v/100)%10);
    tensao_char[3] = '0' + ((tensao_v/10)%10);

    char NNNN[4] = {0};
    NNNN[0] = '0' + ((tensao/1000)%10);
    NNNN[1] = '0' + ((tensao/100)%10);
    NNNN[2] = '0' + ((tensao/10)%10);
    NNNN[3] = '0' + ((tensao/1)%10);

    char mx[4] = {0};
    mx[0] = '0' + ((max/10000)%10);
    mx[1] = '0' + ((max/1000)%10);
    mx[2] = '0' + ((max/100)%10);

    char mn[4] = {0};
    mn[0] = '0' + ((min/10000)%10);
    mn[1] = '0' + ((min/1000)%10);
    mn[2] = '0' + ((min/100)%10);

    lcd_cursor(0x00);
    lcd_char('A');
    lcd_char(modo_char);
    lcd_char('=');
    lcd_char(tensao_char[0]);
    lcd_char(',');
    lcd_char(tensao_char[1]);
    lcd_char(tensao_char[2]);
    lcd_char(tensao_char[3]);
    lcd_char('V');
    lcd_cursor(0xC);
    lcd_char(NNNN[0]);
    lcd_char(NNNN[1]);
    lcd_char(NNNN[2]);
    lcd_char(NNNN[3]);
    lcd_cursor(0x40);
    lcd_char('M');
    lcd_char('n');
    lcd_char('=');
    lcd_char(mn[0]);
    lcd_char(',');
    lcd_char(mn[1]);
    lcd_char(mn[2]);
    lcd_cursor(0x49);
    lcd_char('M');
    lcd_char('x');
    lcd_char('=');
    lcd_char(mx[0]);
    lcd_char(',');
    lcd_char(mx[1]);
    lcd_char(mx[2]);
}

// Buffer circular para armazenas os 10 últimos valores
void buffer_func(int valor){

    if(indexBuffer == BUFFER_SIZE || flag_buffer == 1){
        int i = 0;
        for(i = 0; i < BUFFER_SIZE; i++){
            buffer[i] = buffer[i+1];
        }
        if(flag_buffer == 1) buffer[indexBuffer] = valor;
        indexBuffer = BUFFER_SIZE-1;
        flag_buffer = 1;
    }
    else{
        buffer[indexBuffer] = valor;

        indexBuffer++;
    }
}

// Resetar buffer circular
void reset_buffer(void) {
    int i = 0;
    for(i = 0; i < BUFFER_SIZE; i ++ ){
        buffer[i] = 0;
    }
    indexBuffer = 0;
    flag_buffer = 0;
    flag_change = 0;
}

// Valor máximo dos últimos 10 valores
int max_buffer(void){
    int maxb = 0;
    int i = 0;
    for (i = 0; i < BUFFER_SIZE; i++){
        if (maxb < buffer[i]) maxb = buffer[i];
    }
    return maxb;
}

// Valor mínimo dos últimos 10 valores
int min_buffer(void){
    int minb = 5000;
    int i = 0;
    for (i = 0; i < BUFFER_SIZE; i++){
        if (minb > buffer[i]) minb = buffer[i];
    }
    return minb;
}

// Converter o valor do ADC para V
int converter_adc_volt(int adc_value){
    return ((adc_value*3.3)/4095)*10000;
}

void print_modo3(int modo_t, int tensao_x, int tensao_y){
    // Criar caracteres especiais
    char c0[]={0x00, 0x00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X1F};
    char c1[]={0x00, 0x00, 0X00, 0X00, 0X00, 0X00, 0X1F, 0X00};
    char c2[]={0x00, 0x00, 0X00, 0X00, 0X00, 0X1F, 0X00, 0X00};
    char c3[]={0x00, 0x00, 0X00, 0X00, 0X1F, 0X00, 0X00, 0X00};
    char c4[]={0x00, 0x00, 0X00, 0X1F, 0X00, 0X00, 0X00, 0X00};
    char c5[]={0x00, 0x00, 0X1F, 0X00, 0X00, 0X00, 0X00, 0X00};
    char c6[]={0x00, 0x1F, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00};
    char c7[]={0x1F, 0x00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00};

    // Caracter
    lcd_esp(0,c0);
    lcd_esp(1,c1);
    lcd_esp(2,c2);
    lcd_esp(3,c3);
    lcd_esp(4,c4);
    lcd_esp(5,c5);
    lcd_esp(6,c6);
    lcd_esp(7,c7);

    // Criar vetor auxiliar
    char completo_LCD[32] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
    char posicao_joy = 0;
    int passo = 4096/16;
    int metade = 4096/2;

    // Lógica eixo y
    if (tensao_y < 1*passo) posicao_joy = 7;
    else if (tensao_y < 2*passo) posicao_joy = 6;
    else if (tensao_y < 3*passo) posicao_joy = 5;
    else if (tensao_y < 4*passo) posicao_joy = 4;
    else if (tensao_y < 5*passo) posicao_joy = 3;
    else if (tensao_y < 6*passo) posicao_joy = 2;
    else if (tensao_y < 7*passo) posicao_joy = 1;
    else if (tensao_y < 8*passo) posicao_joy = 0;
    else if (tensao_y < 9*passo) posicao_joy = 7;
    else if (tensao_y < 10*passo) posicao_joy = 6;
    else if (tensao_y < 11*passo) posicao_joy = 5;
    else if (tensao_y < 12*passo) posicao_joy = 4;
    else if (tensao_y < 13*passo) posicao_joy = 3;
    else if (tensao_y < 14*passo) posicao_joy = 2;
    else if (tensao_y < 15*passo) posicao_joy = 1;
    else posicao_joy = 0;

    // Lógica eixo x
    if (tensao_y > metade){
        if (tensao_x < 1*passo) completo_LCD[16] =  posicao_joy;
        else if (tensao_x < 2*passo) completo_LCD[17] =  posicao_joy;
        else if (tensao_x < 3*passo) completo_LCD[18] =  posicao_joy;
        else if (tensao_x < 4*passo) completo_LCD[19] =  posicao_joy;
        else if (tensao_x < 5*passo) completo_LCD[20] =  posicao_joy;
        else if (tensao_x < 6*passo) completo_LCD[21] =  posicao_joy;
        else if (tensao_x < 7*passo) completo_LCD[22] =  posicao_joy;
        else if (tensao_x < 8*passo) completo_LCD[23] =  posicao_joy;
        else if (tensao_x < 9*passo) completo_LCD[24] =  posicao_joy;
        else if (tensao_x < 10*passo) completo_LCD[25] =  posicao_joy;
        else if (tensao_x < 11*passo) completo_LCD[26] =  posicao_joy;
        else if (tensao_x < 12*passo) completo_LCD[27] =  posicao_joy;
        else if (tensao_x < 13*passo) completo_LCD[28] =  posicao_joy;
        else if (tensao_x < 14*passo) completo_LCD[29] =  posicao_joy;
        else if (tensao_x < passo) completo_LCD[30] =  posicao_joy;
        else completo_LCD[31] =  posicao_joy;
    }
    else {
        if (tensao_x < 1*passo) completo_LCD[0] =  posicao_joy;
        else if (tensao_x < 2*passo) completo_LCD[1] =  posicao_joy;
        else if (tensao_x < 3*passo) completo_LCD[2] =  posicao_joy;
        else if (tensao_x < 4*passo) completo_LCD[3] =  posicao_joy;
        else if (tensao_x < 5*passo) completo_LCD[4] =  posicao_joy;
        else if (tensao_x < 6*passo) completo_LCD[5] =  posicao_joy;
        else if (tensao_x < 7*passo) completo_LCD[6] =  posicao_joy;
        else if (tensao_x < 8*passo) completo_LCD[7] =  posicao_joy;
        else if (tensao_x < 9*passo) completo_LCD[8] =  posicao_joy;
        else if (tensao_x < 10*passo) completo_LCD[9] =  posicao_joy;
        else if (tensao_x < 11*passo) completo_LCD[10] =  posicao_joy;
        else if (tensao_x < 12*passo) completo_LCD[11] =  posicao_joy;
        else if (tensao_x < 13*passo) completo_LCD[12] =  posicao_joy;
        else if (tensao_x < 14*passo) completo_LCD[13] =  posicao_joy;
        else if (tensao_x < 15*passo) completo_LCD[14] =  posicao_joy;
        else completo_LCD[15] =  posicao_joy;
    }

    // Printar
    lcd_cursor(0x00);
    lcd_char(completo_LCD[0]);
    lcd_char(completo_LCD[1]);
    lcd_char(completo_LCD[2]);
    lcd_char(completo_LCD[3]);
    lcd_char(completo_LCD[4]);
    lcd_char(completo_LCD[5]);
    lcd_char(completo_LCD[6]);
    lcd_char(completo_LCD[7]);
    lcd_char(completo_LCD[8]);
    lcd_char(completo_LCD[9]);
    lcd_char(completo_LCD[10]);
    lcd_char(completo_LCD[11]);
    lcd_char(completo_LCD[12]);
    lcd_char(completo_LCD[13]);
    lcd_char(completo_LCD[14]);
    lcd_char(completo_LCD[15]);
    lcd_cursor(0x40);
    lcd_char(completo_LCD[16]);
    lcd_char(completo_LCD[17]);
    lcd_char(completo_LCD[18]);
    lcd_char(completo_LCD[19]);
    lcd_char(completo_LCD[20]);
    lcd_char(completo_LCD[21]);
    lcd_char(completo_LCD[22]);
    lcd_char(completo_LCD[23]);
    lcd_char(completo_LCD[24]);
    lcd_char(completo_LCD[25]);
    lcd_char(completo_LCD[26]);
    lcd_char(completo_LCD[27]);
    lcd_char(completo_LCD[28]);
    lcd_char(completo_LCD[29]);
    lcd_char(completo_LCD[30]);
    lcd_char(completo_LCD[31]);
}
