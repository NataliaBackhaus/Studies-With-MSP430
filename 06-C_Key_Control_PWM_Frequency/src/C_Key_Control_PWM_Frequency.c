// P1.5 (TA0.4) <==> P2.4 (TA2.1)
// c  = 1  Hz
// de = 11 %
// f  = 5  Hz
// gh = 76 %
// OBSERVA��O!! Quando for testar as capturas, deixar cada combina��o de S1 e S2 rodar por alguns ms antes de observar os valores, eles podem demorar a atualizar
// Para observar os valores da captura de frequ�ncia e de carga, adicionar as duas vari�veis no "expressions", dar play no circuito, definir a combina��o
// de S1 e S2, esperar um pouco e pausar o circuito. Somente dessa forma os valores de captura v�o ser atualizados. Todos est�o funcionando corretamente,
// com um erro de 1Hz e 1% para menos (o que j� � esperado)

#include <msp430.h> 

#define TRUE    1
#define FALSE   0
#define ABERTA 1    //Chave aberta
#define FECHADA 0   //Chave fechada
#define DBC 10000   //Sugest�o para o debounce

#define POTMAX10HZ 3276     //32768*0,1 - pot�ncia m�xima para f=10Hz
#define POTMAXC 32768       //32768*1 - pot�ncia m�xima para f=1Hz
#define POTMAXF 6553        //32768*0,2 - pot�ncia m�xima para f=5Hz
#define POTMAX4F 1638       //32768*0,05 - pot�ncia m�xima para f=20Hz

int ps1=ABERTA,ps2=ABERTA;

int pot30    = (POTMAX10HZ * 30.0 / 100);   //Programar PWM - c�lculo da carga para 30% e f=10Hz = 982,8
int pot11    = (POTMAXC * 11.0 / 100);      //Programar PWM - c�lculo da carga para 11% e f=1Hz  = 3604,48
int pot76    = (POTMAXF * 76.0 / 100);      //Programar PWM - c�lculo da carga para 76% e f=5Hz  = 4980,28
int pot76_4F = (POTMAX4F * 76.0 / 100);     //Programar PWM - c�lculo da carga para 76% e f=20Hz = 1244,88

int check_s1(void);
int check_s2(void);
void debounce(int valor);
void ta2_config(void);
void ta0_config(void);
void gpio_config(void);

int times[3] = {};                  //Vari�vel para armazenar os valores de captura
int countTimes = 0;
float difFreq = 0;                  //Calcular a diferen�a entre as capturas para descobrir a frequ�ncia
float difCarga = 0;                 //Calcular a diferen�a entre as capturas para descobrir a carga
volatile unsigned int frequencia;   //Frequ�ncia em Hz
volatile unsigned int carga;        //Carga em %
volatile unsigned int flag;         //Coordenar com interrup��o
volatile unsigned int flag_s1;      //Flag auxiliar para chegar a chave S1
volatile unsigned int flag_s2;      //Flag auxiliar para chegar a chave S2

int main(void){
    volatile int pot=30, freq=10;       //iniciar com 30% de pot�ncia e frequ�ncia = 10Hz
    WDTCTL = WDTPW | WDTHOLD;           // stop watchdog timer
    gpio_config();
    ta0_config();
    ta2_config();
    __enable_interrupt();
    while(TRUE){                    //La�o infinito
        while(flag==FALSE);         //Esperar Captura
        flag=FALSE;                 //Prep flag para a pr�xima
        // Monitoramento das chave
        flag_s1 = check_s1();
        flag_s2 = check_s2();
        //C�lculo da frequ�ncia e carga
        if ((ps1 == ABERTA) && (ps2 == ABERTA)){
            TA0CCR0 = POTMAX10HZ;   //Per�odo PWM
            TA0CCR4 = pot30;        //Definir a carga
        }
        else if ((ps1 == ABERTA) && (ps2 == FECHADA)){
            TA0CCR0 = POTMAXC;      //Per�odo PWM
            TA0CCR4 = pot11;        //Definir a carga
        }
        else if ((ps1 == FECHADA) && (ps2 == ABERTA)){
            TA0CCR0 = POTMAXF;      //Per�odo PWM
            TA0CCR4 = pot76;        //Definir a carga
        }
        else if ((ps1 == FECHADA) && (ps2 == FECHADA)){
            TA0CCR0 = POTMAX4F;     //Per�odo PWM
            TA0CCR4 = pot76_4F;     //Definir a carga
        }
    }
    return 0;
}

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
// Debounce
void debounce(int valor){
    volatile int x;             //volatile evita optimizador do compilador
    for (x=0; x<valor; x++);    //Apenas gasta tempo
}

// TA2.1 - Interrup��o de captura
//#pragma vector = TIMER2_A1_VECTOR
// OBSERVA��O!! Quando for testar as capturas, deixar cada combina��o de S1 e S2 rodar por alguns ms antes de observar os valores, eles podem demorar a atualizar
// Para observar os valores da captura de frequ�ncia e de carga, adicionar as duas vari�veis no "expressions", dar play no circuito, definir a combina��o
// de S1 e S2, esperar um pouco e pausar o circuito. Somente dessa forma os valores de captura v�o ser atualizados. Todos est�o funcionando corretamente,
// com um erro de 1Hz e 1% para menos (o que j� � esperado)
#pragma vector = 43
__interrupt void ta2(void){
    TA2IV;                                      //Ler TA2IV para zerar CCIFG
    if(countTimes == 3){
        difFreq = times[2]-times[0];            //Diferen�a entre os valores de captura para ajudar a determinar a frequ�ncia
        difCarga = times[1]-times[0];           //Diferen�a entre os valores de captura para ajudar a determinar a frequ�ncia

        if (difFreq<0) difFreq +=65536L;        //Corrigir caso a diferen�a seja negativa
        if (difCarga<0) difCarga +=65536L;      //Corrigir caso a diferen�a seja negativa

        frequencia = 32768./difFreq;            //Calcular a frequ�ncia

        carga = difCarga/difFreq*100;           //Calcular a carga
        times[0] = times[2];                    //Atualizar valor
        times[1] = TA2CCR1;                     //Atualizar valor
        countTimes = 1;
    }
    else{
        times[countTimes] = TA2CCR1;
    }
    countTimes++;

    flag = TRUE;
}

// Configura��o de TA0.4 (P1.5)
// Condi��o inicial: Freq=10 Hz e carga=30%s
// Esta � situa��o das duas chaves soltas
void ta0_config(void){
    TA0CTL = TASSEL_1 | MC_1;   //TA0 como ACLK e modo 1
    TA0CCTL4 = OUTMOD_6;        //Sa�da Modo 6
    TA0CCR0 = POTMAX10HZ;       //Per�odo PWM
    TA0CCR4 = pot30;            //Iniciar com a primeira situa��o em que S1=S2=A e carga = 30%
}

// (P2.4=TA2.1) Capturar SMCLK/16
// OBSERVA��O! Usei ID_3 e TAIDEX_3 para dividir por 32, visto que a frequ�ncia no segundo caso � 1Hz
void ta2_config(void){
    TA2CTL=TASSEL_2 | MC_2 | ID_3;                  //SMCLK e Modo 2 (cont�nuo)
    TA2CCTL1 = CM_3 | CCIS_0 | SCS | CAP | CCIE;    //Hab. interrup��o
    TA2EX0 = TAIDEX_3;                              //TAIDEX_3 por conta da frequ�ncia c ser 1Hz
    P2DIR &= ~BIT4;                                 //P2.4 = entrada
    P2SEL |= BIT4;                                  //dedicada � captura
}

// Colocada a configura��o de todos os pinos de IO (GPIO)
void gpio_config(void){
    // S1 = entrada com pull-up
    P2DIR &= ~BIT1; //S1 = P2.1 = entrada
    P2REN |=  BIT1; //Habilitar resistor
    P2OUT |=  BIT1; //Habilitar pullup

    // S2 = entrada com pull-up
    P1DIR &= ~BIT1; //S1 = P2.1 = entrada
    P1REN |=  BIT1; //Habilitar resistor
    P1OUT |=  BIT1; //Habilitar pullup

    // TA0.4 pino P1.5
    P1DIR |= BIT5;  //TA0.4 = P1.5
    P1SEL |= BIT5;  //Ligar cabo para Led Vermelho
    }
