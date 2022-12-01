
/*
 *Функции нициализации МК
 */
#include "initchip.h"

//------------------------------------------------------------------------------
/**
 * \file
 * \brief Инициализация регистров  конфигурация портов PIC
 */
//------------------------------------------------------------------------------
extern __eeprom ee_t ee;

void initCore(void) {
    initCPU();
    initANALOG();
    initPORTA();
    initWDT();
}

void initDrive(void) {
    initTMR0();
    initTMR1();
    setPINS();
}
//------------------------------------------------------------------------------

void initCPU(void) {
    // SCS FOSC; SPLLEN disabled; IRCF 16MHz_HF; 
    OSCCON = 0x78; //16 000 000
    // 0x78-16mgz  0x70-8mgz  0xf0-4x8=32mgz pll
    // TUN 0; 
    OSCTUNE = 0x00;
    // SBOREN disabled; 
    BORCON = 0x00;
}
//------------------------------------------------------------------------------

void initANALOG(void) {
    ANSELA = 0x00; // all analog to digital I/O 
}

void initPORTA(void) {
    //init_PORTA
    ANSELA = 0x00; // all analog to digital I/O    
    PORTA = 0x00;
    LATA = 0; //all pins set 0
    TRISA = 0x3F; //all pins input

}
//----------------------------------------------------------------------

/**
 * Инициализация UART
 */

void setPINS(void) {
    //Назначаем ноги Tx и Rx для USART
    APFCON = 0;
    APFCONbits.RXDTSEL = 1; //RxD: 0->RA1  1->RA5
    APFCONbits.TXCKSEL = 1; //TxD: 0->RA0  1->RA4
    //Начальные значения
    PORTAbits.RA4 = 1; //TxD = 1
    PORTAbits.RA0 = 0; //TxE = 0 направление передачи - прием
    //Направление передачи
    TRISAbits.TRISA5 = 1; //RxD -> RA5 input          
    TRISAbits.TRISA4 = 0; //TxD -> RA4 output  
    TRISAbits.TRISA3 = 1; //INT -> RA3 input
    TRISAbits.TRISA2 = 1; //SDA -> RA2 input
    TRISAbits.TRISA1 = 1; //SCL -> RA1 input
    TRISAbits.TRISA0 = 0; //TxE -> RA0 output
}

void initUART(uint8_t speed) {
    TXSTA = 0;
    RCSTA = 0;
    BAUDCON = 0;

    //SYNC=0 BRGH=1 BRG16=1   ->  Fosc/4*[N+1]    
    TXSTAbits.SYNC = 0; //TX Асинхронный режим SYNC
    TXSTAbits.BRGH = 1; //высокоскоростной BRGH
    BAUDCONbits.BRG16 = 1; //BRG16 16-битный генератор скорости
    SPBRGL = speed; //установим делитель скорости
    SPBRGH = 0;

    //TXSTAbits.TXEN = 1; //TX Включить TX передачу
    //RCSTAbits.CREN = 1; //CREN Включить RX прием
    RCSTAbits.SPEN = 1; //SPEN Включить УАРТ1 
}
//----------------------------------------------------------------------

void initTMR0(void) {
    //init_TMR0
    /*Настроим таймер 0 для обеспечения системного тика времени*/
    OPTION_REG = 0b10000000; //Прескалер 1:2, тактирование Fosc/4,
    //подтягивающие резисторы отключены
    //один тик 2000000/125 = 62.5мкс (us)

}
//------------------------------------------------------------------------------

void initTMR1(void) {
    //init_TMR1
    //T1GSS T1G_pin; TMR1GE disabled; T1GTM disabled; T1GPOL low; T1GGO done; T1GSPM disabled; 
    T1GCON = 0x00;
    CCP1CON = 0;
    //TMR1H 248; 
    TMR1H = _TMR1H; //0xF8;
    //TMR1L 48; 
    TMR1L = _TMR1L; //0x30;
    // T1CKPS 1:2; T1OSCEN disabled; nT1SYNC synchronize; TMR1CS FOSC/4; TMR1ON enabled; 
    T1CON = 0x10; // 00(Fosc/4) 01 (1:2), таймер СТОП!

    //OSC = 16 000 000; тактирование Fosc/4 = 16000000/4 = 4 000 000
    //Прескалер 1:2, (Fosc/4 ) / 4 = 4 000 000/2= 2 000 000 (500ns)
    //FFFF-F830 = 7CF(1999dec)
    //один тик 500ns * 2000 = 1мс (ms)
}
//------------------------------------------------------------------------------


//  Инициализация регистроов MSSP

/**
 * \brief  Инициализация регистроов MSSP
 * 
 */

void initI2C(void) {
    SSP1STAT = 0x00;
    SSP1CON1 = 0x08;
    SSP1CON2 = 0x00;
    SSP1ADD = 39; //100kgz=Fi2c = Fosc/(4*(SSP1ADD+1));
    // 16000000/(4*(39+)) = 100000
    SSP1CON1bits.SSPEN = 1;
}

void initWDT(void) {
    //init_WDT

    WDTCON = 0b00001001; //16384x(1/31000)=512ms
    //        WDTCONbits.SWDTEN=1;
}

void initINT(void) {
    //Разрешаем прерывание от Таймера 0
    INTCONbits.TMR0IF = 0;
    INTCONbits.TMR0IE = 1;
    //Разрешаем прерывание от Таймера 1
    PIR1bits.TMR1IF = 0;
    PIE1bits.TMR1IE = 1;
    //Разрешаем прерывание от RA3
    INTCONbits.IOCIF = 0;
    //   INTCONbits.IOCIE = 1; 
    IOCAP = 0; //rasing edge
    IOCAN = 0; //falling edge
    IOCAF = 0; //
    //Разрешаем прерывание от RA3 по падающему фронту
    IOCANbits.IOCAN3 = 1;
    //Разрешаем прерывание от I2C
    PIR1bits.SSP1IF = 0;
    PIE1bits.SSP1IE = 1;
    //Разрешение прерывание от приемника УАРТ
    PIE1bits.RCIE = 1;
    INTCONbits.PEIE = 1; //Разрешаем прерывания от переферии
    INTCONbits.GIE = 1; //разрешаетвысокоприоритетные прерывания

}
