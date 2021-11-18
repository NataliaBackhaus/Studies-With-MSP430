#include <msp430.h> 

#define TRUE 1
#define FALSE 0
#define ABERTA 1 //Chave aberta
#define FECHADA 0 //Chave fechada
#define DBC 10000 //Sugestão para o debounce

volatile char vm[] = {3, 5};    // Exemplo para controle led vermelho
volatile char vd[] = {4, 2};    // Exemplo para controle led verde

int flag_vm = 0;                //flag = 0 > led vermelho apagado; flag = 1 > led vermelho ligado
int flag_vd = 0;                //flag = 0 > led verde apagado; flag = 1 > led verde ligado

int mon_s1(void);
int mon_s2(void);
void debounce(int valor);
void io_config(void);
inline void led_vd(void);
inline void led_VD(void);
inline void led_vm(void);
inline void led_VM(void);

int main(void){
    int vm_cont = 0;            //contador para led vermelho
    int vd_cont = 0;            //contador para led verded
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer
    io_config();
    while (TRUE){
        if (mon_s1() == TRUE) vm_cont++; //Chave 1 acionada, aumenta o contador vm
        if (mon_s2() == TRUE) vd_cont++; //Chave 2 acionada, aumenta o contador vd
        //Se o led vermelho estiver apagado e o contador for igual ao primeiro valor de vm[]
        if ((vm_cont == vm[0]) &&(flag_vm == 0))
            {
            led_VM();       //chama função para ligar o led vermelho
            flag_vm = 1;    //muda flag para ligado
            vm_cont = 0;    //zera o contador
            }
        //Se o led vermelho estiver ligado e o contador for igual ao segundo valor de vm[]
        if ((vm_cont == vm[1]) &&(flag_vm == 1))
                    {
                    led_vm();       //chama função para desligar o led vermelho
                    flag_vm = 0;    //muda flag para apagado
                    vm_cont = 0;    //zera contador
                    }
        //Se o led verde estiver apagado e o contador for igual ao primeiro valor de vd[]
        if ((vd_cont == vd[0]) &&(flag_vd == 0))
                    {
                    led_VD();       //chama função para ligar o led verde
                    flag_vd = 1;    //muda flag para ligado
                    vd_cont = 0;    //zera o contador
                    }
        //Se o led verde estiver ligado e o contador for igual ao segundo valor de vd[]
        if ((vd_cont == vd[1]) &&(flag_vd == 1))
                            {
                            led_vd();       //chama função para desligar o led verde
                            flag_vd = 0;    //muda flag para apagado
                            vd_cont = 0;    //zera o contador
                            }

    }
    return 0;
}
// Monitorar S1, retorna TRUE se foi acionada
int mon_s1(void){
    static int ps1=ABERTA; //Guardar passado de S1
    if ((P2IN&BIT1) == 0){ //Qual estado atual de S1?
        if (ps1==ABERTA){ //Qual o passado de S1?
            debounce(DBC);
            ps1=FECHADA;
            return TRUE;
       }
    }
    else{
        if (ps1==FECHADA){ //Qual o passado de S1?
            debounce(DBC);
            ps1=ABERTA;
            return FALSE;
        }
    }
    return FALSE;
}
// Monitorar S2, retorna TRUE se foi acionada

int mon_s2(void){
    static int ps2=ABERTA; //Guardar passado de S2
    if ((P1IN&BIT1) == 0){ //Qual estado atual de S2?
        if (ps2==ABERTA){ //Qual o passado de S2?
            debounce(DBC);
            ps2=FECHADA;
            return TRUE;
        }
    }
    else{
        if (ps2==FECHADA){ //Qual o passado de S2?
            debounce(DBC);
            ps2=ABERTA;
            return FALSE;
        }
    }
    return FALSE;
}

// Acionar os leds

inline void led_VM(void) {P1OUT |=  BIT0;}  //Acende vermelho
inline void led_vm(void) {P1OUT &= ~BIT0;}  //Apaga vermelho

inline void led_VD(void) {P4OUT |=  BIT7;}  //Acende verde
inline void led_vd(void) {P4OUT &= ~BIT7;}  //Apaga verde

// Debounce
void debounce(int valor){
    volatile int x; //volatile evita optimizador do compilador
    for (x=0; x<valor; x++); //Apenas gasta tempo
}

// Configurar GPIO
void io_config(void){
    P1DIR |=  BIT0; //Led1 = P1.0 = saída
    P1OUT &= ~BIT0; //Led1 apagado
    P4DIR |=  BIT7; //Led2 = P4.7 = saída
    P4OUT &= ~BIT7; //Led1 apagado
    P2DIR &= ~BIT1; //S1 = P2.1 = entrada
    P2REN |=  BIT1; //Habilitar resistor
    P2OUT |=  BIT1; //Habilitar pullup
    P1DIR &= ~BIT1; //S1 = P2.1 = entrada
    P1REN |=  BIT1; //Habilitar resistor
    P1OUT |=  BIT1; //Habilitar pullup
}
