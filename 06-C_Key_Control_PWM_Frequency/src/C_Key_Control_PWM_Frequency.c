// P1.5 (TA0.4) <==> P2.4 (TA2.1)
// c  = 1  Hz
// de = 11 %
// f  = 5  Hz
// gh = 76 %
// OBSERVAÇÃO!! Quando for testar as capturas, deixar cada combinação de S1 e S2 rodar por alguns ms antes de observar os valores, eles podem demorar a atualizar
// Para observar os valores da captura de frequência e de carga, adicionar as duas variáveis no "expressions", dar play no circuito, definir a combinação
// de S1 e S2, esperar um pouco e pausar o circuito. Somente dessa forma os valores de captura vão ser atualizados. Todos estão funcionando corretamente,
// com um erro de 1Hz e 1% para menos (o que já é esperado)

#include <msp430.h> 

#define TRUE    1
#define FALSE   0
#define ABERTA 1    //Chave aberta
#define FECHADA 0   //Chave fechada
#define DBC 10000   //Sugestão para o debounce

#define POTMAX10HZ 3276     //32768*0,1 - potência máxima para f=10Hz
#define POTMAXC 32768       //32768*1 - potência máxima para f=1Hz
#define POTMAXF 6553        //32768*0,2 - potência máxima para f=5Hz
#define POTMAX4F 1638       //32768*0,05 - potência máxima para f=20Hz

int ps1=ABERTA,ps2=ABERTA;

int pot30    = (POTMAX10HZ * 30.0 / 100);   //Programar PWM - cálculo da carga para 30% e f=10Hz = 982,8
int pot11    = (POTMAXC * 11.0 / 100);      //Programar PWM - cálculo da carga para 11% e f=1Hz  = 3604,48
int pot76    = (POTMAXF * 76.0 / 100);      //Programar PWM - cálculo da carga para 76% e f=5Hz  = 4980,28
int pot76_4F = (POTMAX4F * 76.0 / 100);     //Programar PWM - cálculo da carga para 76% e f=20Hz = 1244,88

int check_s1(void);
int check_s2(void);
void debounce(int valor);
void ta2_config(void);
void ta0_config(void);
void gpio_config(void);

int times[3] = {};                  //Variável para armazenar os valores de captura
int countTimes = 0;
float difFreq = 0;                  //Calcular a diferença entre as capturas para descobrir a frequência
float difCarga = 0;                 //Calcular a diferença entre as capturas para descobrir a carga
volatile unsigned int frequencia;   //Frequência em Hz
volatile unsigned int carga;        //Carga em %
volatile unsigned int flag;         //Coordenar com interrupção
volatile unsigned int flag_s1;      //Flag auxiliar para chegar a chave S1
volatile unsigned int flag_s2;      //Flag auxiliar para chegar a chave S2

int main(void){
    volatile int pot=30, freq=10;       //iniciar com 30% de potência e frequência = 10Hz
    WDTCTL = WDTPW | WDTHOLD;           // stop watchdog timer
    gpio_config();
    ta0_config();
    ta2_config();
    __enable_interrupt();
    while(TRUE){                    //Laço infinito
        while(flag==FALSE);         //Esperar Captura
        flag=FALSE;                 //Prep flag para a próxima
        // Monitoramento das chave
        flag_s1 = check_s1();
        flag_s2 = check_s2();
        //Cálculo da frequência e carga
        if ((ps1 == ABERTA) && (ps2 == ABERTA)){
            TA0CCR0 = POTMAX10HZ;   //Período PWM
            TA0CCR4 = pot30;        //Definir a carga
        }
        else if ((ps1 == ABERTA) && (ps2 == FECHADA)){
            TA0CCR0 = POTMAXC;      //Período PWM
            TA0CCR4 = pot11;        //Definir a carga
        }
        else if ((ps1 == FECHADA) && (ps2 == ABERTA)){
            TA0CCR0 = POTMAXF;      //Período PWM
            TA0CCR4 = pot76;        //Definir a carga
        }
        else if ((ps1 == FECHADA) && (ps2 == FECHADA)){
            TA0CCR0 = POTMAX4F;     //Período PWM
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

// TA2.1 - Interrupção de captura
//#pragma vector = TIMER2_A1_VECTOR
// OBSERVAÇÃO!! Quando for testar as capturas, deixar cada combinação de S1 e S2 rodar por alguns ms antes de observar os valores, eles podem demorar a atualizar
// Para observar os valores da captura de frequência e de carga, adicionar as duas variáveis no "expressions", dar play no circuito, definir a combinação
// de S1 e S2, esperar um pouco e pausar o circuito. Somente dessa forma os valores de captura vão ser atualizados. Todos estão funcionando corretamente,
// com um erro de 1Hz e 1% para menos (o que já é esperado)
#pragma vector = 43
__interrupt void ta2(void){
    TA2IV;                                      //Ler TA2IV para zerar CCIFG
    if(countTimes == 3){
        difFreq = times[2]-times[0];            //Diferença entre os valores de captura para ajudar a determinar a frequência
        difCarga = times[1]-times[0];           //Diferença entre os valores de captura para ajudar a determinar a frequência

        if (difFreq<0) difFreq +=65536L;        //Corrigir caso a diferença seja negativa
        if (difCarga<0) difCarga +=65536L;      //Corrigir caso a diferença seja negativa

        frequencia = 32768./difFreq;            //Calcular a frequência

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

// Configuração de TA0.4 (P1.5)
// Condição inicial: Freq=10 Hz e carga=30%s
// Esta é situação das duas chaves soltas
void ta0_config(void){
    TA0CTL = TASSEL_1 | MC_1;   //TA0 como ACLK e modo 1
    TA0CCTL4 = OUTMOD_6;        //Saída Modo 6
    TA0CCR0 = POTMAX10HZ;       //Período PWM
    TA0CCR4 = pot30;            //Iniciar com a primeira situação em que S1=S2=A e carga = 30%
}

// (P2.4=TA2.1) Capturar SMCLK/16
// OBSERVAÇÃO! Usei ID_3 e TAIDEX_3 para dividir por 32, visto que a frequência no segundo caso é 1Hz
void ta2_config(void){
    TA2CTL=TASSEL_2 | MC_2 | ID_3;                  //SMCLK e Modo 2 (contínuo)
    TA2CCTL1 = CM_3 | CCIS_0 | SCS | CAP | CCIE;    //Hab. interrupção
    TA2EX0 = TAIDEX_3;                              //TAIDEX_3 por conta da frequência c ser 1Hz
    P2DIR &= ~BIT4;                                 //P2.4 = entrada
    P2SEL |= BIT4;                                  //dedicada à captura
}

// Colocada a configuração de todos os pinos de IO (GPIO)
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
