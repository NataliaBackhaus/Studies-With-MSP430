// Gerar pulso de Trigger com SMCLK
// Trig = P1.5 = TA0.4

// Capturar echo por P2.0 (TA1.1)

#include <msp430.h>
#include <stdint.h>

#define TRUE    1
#define FALSE   0
#define ABERTA  1       //Chave aberta
#define FECHADA 0       //Chave fechada
#define DBC     1000    //Sugestão para o debounce

void gpio_config();
void ta1_config(void);
void ta0_config(void);
void leds(long distancia);
int calc_dist(long aux);
int calc_freq(long dist);
void ta2_prog(int freq);
int check_s1(void);
int check_s2(void);
void debounce(int valor);

int ps1=ABERTA,ps2=ABERTA;

#define TOPO 52432  //Topo da contagem = 50ms
#define TRIG  21    //Larg Trig = 20 us

//Frequência do SMCLK
#define FREQSMCLK   1048576         // Frequência adotada para o SMCLK

//Definir variáveis globais auxiliares
long dist;
long aux;

void main(void){
    volatile long freq;
    volatile int estadoChave=0;
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    gpio_config();
    ta1_config();
    ta0_config();
    __enable_interrupt();

    while(1){
        __low_power_mode_0();   //Esperar flanco de subida
        __low_power_mode_0();   //Esperar flanco de descida
        dist = calc_dist(aux);  //Calcular distância
        leds(dist);             //Acender leds em função da distância
        check_s1();             //Monitorar chave S1
        check_s2();             //Monitorar chave S2
        freq = calc_freq(dist); //Calcular a frequência a ser gerada
        ta2_prog(freq);         //Programar TA2 de acordo com a frequência
    }
}

void ta1_config(void){
    TA1CTL = TASSEL_2 | MC_2 | TACLR;           //TA1 como SMCLK e modo 2
    TA1CCTL1 = CM_3 | CCIS_0 | CAP | CCIE;      //Configurar captura
}

void ta0_config(void){
    TA0CTL = TASSEL_2 | MC_1 | TACLR;           //TA0 como SMCLK e modo 2
    TA0CCR0 = TOPO;                             //Definir limite máximo de TA0CCR0
    TA0CCTL4 = OUTMOD_7;                        //Output 7
    TA0CCR4 = TRIG;                             //Definir valor do trigger
}

void gpio_config(){
    //Configurar P1.5 > TA0.4
    P1DIR |= BIT5;              //Configurar como saída
    P1SEL |= BIT5;              //TA0.4

    //Configurar P2.0 > TA1.1
    P2DIR &= ~BIT0;             //Configurar como entrada
    P2SEL |= BIT0;              //TA1.1

    //Configurar P2.5 > TA2.2
    P2DIR |= BIT5;              //Configurar como saída
    P2SEL |= BIT5;              //TA2.2

    //Configurar LED vermelho
    P1DIR |= BIT0;              //Configurar como saída
    P1OUT &= ~BIT0;             //Inicializando apagado

    //Configurar LED verde
    P4DIR |= BIT7;              //Configurar como saída
    P4OUT &= ~BIT7;             //Inicializando apagado

}

int calc_dist(long aux){
    //Cálculo para obtenção da distância
    dist = (aux*34000)/2;
    dist >>= 20;
    return dist;
}

void leds(long distancia){
    //Obstáculo < 10 cm
    if(distancia <= 10){
        P4OUT |= BIT7;         //Acende led verde
        P1OUT |= BIT0;         //Acende led vermelho
    }

    //10 cm < obstáculo < 30 cm
    else if((distancia > 10)&(distancia <= 30)){
        P4OUT &= ~BIT7;        //Apaga led verde
        P1OUT |= BIT0;         //Acende led vermelho
    }

    //30 cm < obstáculo < 50 cm
    else if((distancia > 30)&(distancia <= 50)){
        P4OUT |= BIT7;         //Acende led verde
        P1OUT &= ~BIT0;        //Apaga led vermelho
    }

    //Obstáculo > 50 cm
    else{
        P4OUT &= ~BIT7;        //Apaga led verde
        P1OUT &= ~BIT0;        //Apaga led vermelho
    }
}

int calc_freq(long dist){
    //Se chaves estão abertas, frequência = -1*distancia + 50  (y=ax+b)
    //Se qualquer chave está fechada, frequência = 1*distancia + 0 (y=ax+b)
    int a = -1;
    int b = 50;

    if ((ps1 == ABERTA) && (ps2 == ABERTA)){
        a = -1;
        b = 50;
    }
    else {
        a = 1;
        b = 0;
    }

    if(dist > 50)                   //Se distância é maior que 50 cm, não produz som
        return 0;
    else                            //Se distância está entre 0 e 50 cm, calcula a frequência
        return (dist*a + b)*100;    //Multiplicar por 100
}

void ta2_prog(int freq){
    TA2CTL = TASSEL__SMCLK | MC__UP | TACLR;
    TA2CCR0 = FREQSMCLK/freq;
    TA2CCR2 = TA2CCR0 >> 1;
    TA2CCTL2 = OUTMOD_3;
}

#pragma vector = TIMER1_A1_VECTOR
__interrupt void TA1_CCRN_ISR(){
    static long fs, fd, cont = 0;
    //fs --> recebe o valor do flanco de subida do pulso do echo
    //fd --: recebe o valor do flanco de descida do pulso do echo
    if (TA1IV == 0x2){
        if(cont == 0){                  //cont = 0 > flanco de subida
            fs = TA1CCR1;
            cont++;                     //Incrementa contador
        }
        else{                           //cont diferente de 0 > flanco de descida
            fd = TA1CCR1;
            aux = fd - fs;              //largura do pulso
            cont = 0;
        }
        __low_power_mode_off_on_exit();
    }
}

//Checar chave S2
int check_s2(void){
    if ((P1IN&BIT1) == 0){ //S1 fechada
        if (ps2==ABERTA){ //A --> F
            debounce(DBC);
            ps2=FECHADA; //Guardar o passado
            return TRUE;
        }
        else
            return FALSE; //F --> F
        }
    else{ //S1 aberta
        if (ps2==ABERTA)
            return FALSE; //A --> A
        else{
            debounce(DBC);
            ps2=ABERTA; //Guardar o passado
            return FALSE; //F --> A
        }
    }
}

//Checar chave S1
int check_s1(void){
    if ((P2IN&BIT1) == 0){ //S1 fechada
        if (ps1==ABERTA){ //A --> F
            debounce(DBC);
            ps1=FECHADA; //Guardar o passado
            return TRUE;
        }
        else
            return FALSE; //F --> F
        }
    else{ //S1 aberta
        if (ps1==ABERTA)
            return FALSE; //A --> A
        else{
            debounce(DBC);
            ps1=ABERTA; //Guardar o passado
            return FALSE; //F --> A
        }
    }
}

//Debounce
void debounce(int valor){
    volatile int x;             //volatile evita otimizador do compilador
    for (x=0; x<valor; x++);    //Apenas gasta tempo
}
