// De 0% a 100% em 5 seg (5 passos de 20%)
// De 100% a 0% em 5 seg (5 passos de 20%)
// TB0.2 (P7.4) --> cabo --> Led 1 (vermelho)
// TB0.1 --> mapeamento --> P4.7 = Led 2 (verde)
// S1 e S2 controlam o PWM
// Timer B0 = 1.048.576 Hz
// Frequência do PWM é de 128Hz
// TB0CCR0 = 1.048.576*1/128 -1 = 8191
// 20% = 1638,2 -> arredondado para 1638


#include <msp430.h>

#define TRUE    1
#define FALSE   0
#define ABERTA 1    //Chave aberta
#define FECHADA 0   //Chave fechada
#define DBC 10000   //Sugestão para o debounce

#define PASSO 1638          //Cálculo dos passos feito na linha 12
#define POT100 (5*PASSO)    //Potência = 100%

void tb0_config(void);
void gpio_config(void);
int check_s2(void);
int check_s1(void);
void debounce(int valor);
void passo_vd(void);
void passo_vm(void);

volatile int cont;      //contar as interrupções
volatile int fase_vm;   //Sentido do incremento do LED vermelho
volatile int fase_vd;   //Sentido do incremento do LED verde
                        //Fase=0 -> aumenta brilho
                        //Fase=1 -> diminui brilho

volatile unsigned int flag_s1;  //Flag para ajudar a monitorar o estado de S1
volatile unsigned int flag_s2;  //Flag para ajudar a monitorar o estado de S2
int ps1=ABERTA,ps2=ABERTA;      //Estado anterior das chaves

int main(void){
    WDTCTL = WDTPW | WDTHOLD;   //Stop watchdog timer
    gpio_config();              //configurar GPIO
    tb0_config();               //configurar tb0
    __enable_interrupt();       //habilitar interrupções
    while (TRUE){
        flag_s1 = check_s1();   //Checar flag de S1
        flag_s2 = check_s2();   //Checar flag de S2

        if((ps1 == FECHADA) && (flag_s2 == TRUE)){
            passo_vd();
        }

        if((ps2 == FECHADA) && (flag_s1 == TRUE)){
            passo_vm();
        }
    }
    return 0;
}

// TB0 - Sua rotina de Interrupção, escolher opções
//#pragma vector = TIMER_B0_VECTOR
#pragma vector = 59
//#pragma vector = TIMER_B1_VECTOR
//#pragma vector = 58
__interrupt void tb0ccr0(void) {
    if((ps1 == ABERTA) && (ps2 == ABERTA)){
        if(++cont == 128){ //Contar 5 ms
            cont=0;
            passo_vd();
            passo_vm();
        }
    }
}

void passo_vd(void){
    if (fase_vd == 0){          //Se LED verde está subindo
        if (TB0CCR1 == POT100){ //Se a potência do LED verde chegou em 100%
            fase_vd = 1;        //Muda o sentido de LED verde para descida
            TB0CCR1 -= PASSO;   //Diminui brilho
        }
        else {
            TB0CCR1 += PASSO;   //Incrementa brilho
        }
    }
    else if (fase_vd == 1){     //Se LED verde está descendo
        if (TB0CCR1 == 0){      //Se a potência do LED verde chegou em 0
            fase_vd = 0;        //Muda o sentido de LED verde para subida
            TB0CCR1 += PASSO;   //Incrementa brilho
        }
        else {
            TB0CCR1 -= PASSO;   //Diminui brilho
        }
    }
}

void passo_vm(void){
    if (fase_vm == 0){          //Se LED verde está subindo
        if (TB0CCR2 == POT100){ //Se a potência do LED vermelho chegou em 100%
            fase_vm = 1;        //Muda o sentido de LED vermelho para descida
            TB0CCR2 -= PASSO;   //Diminui brilho
        }
        else {
            TB0CCR2 += PASSO;   //Aumenta brilho
        }
    }
    else if (fase_vm == 1){     //Se LED vermelho está descendo
        if (TB0CCR2 == 0){      //Se a potência do LED vermelho chegou em 0
            fase_vm = 0;        //Muda o sentido de LED vermelho para subida
            TB0CCR2 += PASSO;   //Aumenta brilho
        }
        else {
            TB0CCR2 -= PASSO;   //Diminui brilho
        }
    }
}

// Configuração de TB0
void tb0_config(void) {
    TB0CTL = TBSSEL_2 | MC_1;       //TB0 com SMCLK e Modo Up
    TB0CCR0 = POT100;               //Período PWM
    TB0CCTL0 = CCIE;                //Interrup TB0CCR0 CCIFG (vetor 59)
    TB0CCTL1 = CLLD_1 | OUTMOD_6;   //Saída Modo 6
    TB0CCTL2 = CLLD_1 | OUTMOD_6;   //Saída Modo 6
    TB0CCR1 = 0;                    //Definir led verde = 0%
    TB0CCR2 = 0;                    //Definir led vermelhor = 0%
    fase_vm = 0;                    //Definir fase inicial led verde = 0
    fase_vd = 0;                    //Definir fase inicial led vermelho = 0
}

// Configurar GPIO
void gpio_config(void){
    //Configurar pino 7.4
    P7DIR |= BIT4;      //TB0.2 = P7.4
    P7SEL |= BIT4;      //Ligar cabo para Led Vermelho

    // Mapeamento de TB0.1 ==> P4.7
    P4DIR |= BIT7;          //P4.7 = Saída
    P4SEL |= BIT7;          //P4.7 (LED2) saída alternativa
    PMAPKEYID = 0X02D52;    //Liberar mapeamento
    P4MAP7 = PM_TB0CCR1A;   //TB0.1 sair por P4.7 (LED2)

    // S1 = entrada com pull-up
    P2DIR &= ~BIT1; //S1 = P2.1 = entrada
    P2REN |=  BIT1; //Habilitar resistor
    P2OUT |=  BIT1; //Habilitar pullup

    // S2 = entrada com pull-up
    P1DIR &= ~BIT1; //S1 = P2.1 = entrada
    P1REN |=  BIT1; //Habilitar resistor
    P1OUT |=  BIT1; //Habilitar pullup
}



void debounce(int valor){
    volatile int x;             //volatile evita optimizador do compilador
    for (x=0; x<valor; x++);    //Apenas gasta tempo
}


int check_s2(void){
    if ((P1IN&BIT1) == 0){  //S1 fechada
        if (ps2==ABERTA){   //A --> F
            debounce(DBC);
            ps2=FECHADA;    //Guardar o passado
            return TRUE;
        }
        else
            return FALSE;   //F --> F
        }
    else{                   //S1 aberta
        if (ps2==ABERTA)
            return FALSE;   //A --> A
        else{
            debounce(DBC);
            ps2=ABERTA;     //Guardar o passado
            return FALSE;   //F --> A
        }
    }
}

int check_s1(void){
    if ((P2IN&BIT1) == 0){  //S1 fechada
        if (ps1==ABERTA){   //A --> F
            debounce(DBC);
            ps1=FECHADA;    //Guardar o passado
            return TRUE;
        }
        else
            return FALSE;   //F --> F
        }
    else{                   //S1 aberta
        if (ps1==ABERTA)
            return FALSE;   //A --> A
        else{
            debounce(DBC);
            ps1=ABERTA;     //Guardar o passado
            return FALSE;   //F --> A
        }
    }
}
