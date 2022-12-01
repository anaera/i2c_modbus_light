/*
 * File:   actionI2C.c
 * Author: 1
 *
 * Created on 24 мая 2022 г., 19:44
 */



#include "config.h"
#include "initchip.h"
#include "masterI2C.h"
#include "modbus.h"
#include "timers.h"

extern volatile i2c_states_t i2cState;
extern volatile uint8_t regAct; //маска активных регистров
extern uint8_t regMask; //маска текущего регистра
extern uint8_t rdMask; //маска регистров чтения
extern uint8_t regCnt; //текущий регистр
extern uint8_t regALL; //всего регистров
extern uint8_t regINP; //регистров in
extern uint8_t regOUT; //регистров Out
extern core_data_t coreData[INP_REG]; //рабочая область для debounce регистров (макс 8)
extern core_i2c_t wrkReg;
extern __eeprom ee_t ee;
extern COMMON_DATA COMMON;
//extern TIMERS_DATA TIMERS;
extern uint8_t timers[8 * TM_REG];

extern void (*loopFunc)(void);
extern void (*loopStep)(void);

void startLoop(void);
void I2C_MasterWrite(void);
void I2C_MasterRead(void);
void initLoop(void);
void checkLoop(void);
void actionLoop(void);
void checkTail(void);
void exitI2C(void);
void idleLoop(void);
static inline void debounce(void);
void lookActInpBit(uint8_t actBits);
void checkTimer(uint8_t *ptr);

void startLoop(void) {
    regCnt = 0;
    loopFunc = initLoop;
    loopStep = initLoop;
}

void initLoop(void) {
    if (regCnt < regALL) { // все регистры обработали
        wrkReg.addr = ee._addrReg[regCnt]; //берем адрес I2C
        wrkReg.data = ee._dataReg[regCnt]; //берем данные I2C
        COMMON.registrTable[regCnt] = wrkReg.data;
        loopFunc = idleLoop;
        I2C_Write(); //запись регистра I2C
        regCnt++;
    } else { //да
        regCnt = 0;
        regMask = 1;
        //        loopFunc = initLoop;
        //        loopStep = initLoop;
        loopFunc = checkLoop;
        loopStep = checkTail;
        startTimers();
    }
}

/*
void debounceStep(void) {
    
    //.....
    checkTail();
};
 */
void checkLoop(void) {

    if (regAct & regMask) {
        //        regAct = regAct ^ regMask; //сбрасываем маску
        loopFunc = idleLoop;
        wrkReg.addr = ee._addrReg[regCnt]; //берем адрес I2C 
        if (regCnt < regINP) {
            loopStep = actionLoop;
            I2C_Read(); //чтение регистра I2C
            return;
        } else {
            loopStep = checkTail;
            wrkReg.data = COMMON.registrTable[regCnt]; //берем данные I2C 
            I2C_Write(); //запись регистра I2C

            return;
        }
    }
    loopFunc = checkTail;
}

void actionLoop(void) {
    debounce();
    checkTail();
}

void checkTail(void) {
    loopFunc = checkLoop;
    regMask = (uint8_t) (regMask << 1);
    regCnt++;
    if (regCnt < regALL) {
        return;
    }
    //    PORT_TxE = ~PORT_TxE;
    regCnt = 0;
    regMask = 1;
    regAct = 0xFF;
    loopFunc = idleLoop;
};

void exitI2C(void) {
    //    loopFunc = initLoop;
    loopFunc = loopStep;
}

void idleLoop(void) {
    NOP();
    NOP();
    return;
}

void initI2Cbus(void) {
    regINP = ee._rInp;
    regOUT = ee._rOut;
    regALL = regINP + regOUT;
    regMask = 1;
    regAct = 0xFF;
    rdMask = (uint8_t) (1 << regINP) - 1;
    i2cState = I2C_RESET;
    initI2C();
}

static inline void debounce(void) {
    uint8_t val;

    val = wrkReg.data;
    val ^= COMMON.regINP[regCnt]; // какие изменились биты 
    // здесь храним дребезг
    coreData[regCnt].cnt0 = ~(coreData[regCnt].cnt0 & val);
    coreData[regCnt].cnt1 = coreData[regCnt].cnt0 ^ (coreData[regCnt].cnt1 & val);
    //вычленяем очищенное от дребезга, стабильно 4 интервала    
    val &= coreData[regCnt].cnt0 & coreData[regCnt].cnt1;

    COMMON.regINP[regCnt] ^= val; //изменяем биты которые уже без дребезга  !!!!
    if (val) lookActInpBit(val);
    //есть еще дребезжащие нажатия?    
    if ((coreData[regCnt].cnt0 & coreData[regCnt].cnt1) == 0xFF) {
        regAct = regAct&~regMask; // нет, очищаем бит регистра с активными кнопками 
    } else {
        regAct = regAct | regMask; //есть, будем читать этот регистр
    };

    return;
}

void main(void) {

    NOP();
    NOP();
    //    regCnt = 0;
    //    lookActInpBit(0xff);
    //    regCnt = 1;
    //    lookActInpBit(0xff);
    //    regCnt = 2;
    //   lookActInpBit(0xff);

    initCore();
    initDrive();

    initModbus();

    initI2Cbus();

    initINT();

    NOP();
    loopFunc = startLoop;
    //PORT_TxE = 1;
    while (1) {
        if (TIMERS.flagTimers.SECFLG == 1) {
            TIMERS.flagTimers.SECFLG = 0;
            checkTimer(timers);
        }
        engineModbus();
        NOP();
        NOP();
        di();
        loopFunc();
        ei();
        NOP();
        NOP();
        CLRWDT();
    };
    I2C_Close();

};

