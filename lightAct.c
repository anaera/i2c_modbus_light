/*
 * File:   newmain.c
 * Author: Papa
 *
 * Created on 12 ??????? 2022 ?., 21:31
 */


#include <xc.h>
#include <stdint.h>
#include "const.h"
#include "timers.h"

extern __eeprom ee_t ee;
extern uint8_t timers[8 * TM_REG];
extern uint8_t regCnt;
extern COMMON_DATA COMMON;

uint8_t bitCnt, bitMask;
//Массив таймеров от 0 до 8*(кол-во out-регистров) - (16)
//Масси out-регистов от (ee.Inp) до (ee._rInp + ee._nTimers)
//-----------------------------------------------------------------------------

void checkTimer(uint8_t *ptr) { //Массив таймеров на входе
    uint8_t cntBit, mask, regPtr, regExit;
    regExit = (ee._rInp + ee._nTimers); //номер последнего out-регистра
    for (regPtr = ee._rInp; regPtr < regExit; regPtr++) { //перебираем out-регистры
        mask = 1;
        cntBit = 8;
        NOP();
        while (cntBit) { //проходим по всем битам регистра
            if ((*ptr) > 0) {
                (*ptr)--;
                if ((*ptr) == 0) { //таймер обнулился. Единичим бит.
                    COMMON.regALL[regPtr] = COMMON.regALL[regPtr] | mask; //!
                    //  oldBits = oldBits & (~mask);
                }
            }
            cntBit--; //следующий бит
            ptr++;
            mask = (uint8_t) (mask << 1);
        };
    };
    return;
};

//-----------------------------------------------------------------------------
//

void doAct(uint8_t num) {
    uint8_t reg, cnt, mask;
    reg = (uint8_t) (num >> 3) + ee._rInp; //номер out-регистров(глобальный)
    cnt = num & 0x07; //номер бита out-регистра 0-7
    mask = (uint8_t) (1 << cnt);
    cnt = ((uint8_t) (ee._rInp << 3)); //переопределили cnt
    cnt = num + cnt; //номер начального значения таймера (Нумерация сквозная)

    if (ee.flagPress[regCnt] & bitMask) //тип управления -  PRESS/UPDATE !
    {
        if ((COMMON.regINP[regCnt] & bitMask) == 0) { //!!
            //если нажато то переключаем

            if (COMMON.regALL[reg] & mask) { //если было вкл, то выкл (единицей))
                COMMON.regALL[reg] = COMMON.regALL[reg] & (~mask); // включаем !!
                timers[num] = ee.valTimer[cnt]; // запустить таймер
                               
            } else { //иначе вкл
                COMMON.regALL[reg] = COMMON.regALL[reg] | mask; //выключаем !!
                                 timers[num] = 0; // остановить таймер

            }
        }
    } else {
        //        if (ee.out[reg].flagManual & mask) { //!!
        if (ee.flagManual[reg] & mask) //ручное управление ?
        {
            return; //ДА!
        }
        //НЕТ!  автомат , далее
        //        if ((ee.out[reg].flagNight & mask) && dayFlag) {//!!
        if ((ee.flagNight[reg] & mask)) { //&& time.flags.dayFlag) { ///????????????
            return; //если только ночью и сейчас день


        }
        //переустановили таймер и обновили regOUT
        //        timers[num] = ee.valTimer[cnt]; // cnt - сквозная нумерация
                COMMON.regALL[reg] = COMMON.regALL[reg] & (~mask); // включаем !!
                        timers[num] = ee.valTimer[cnt]; // запустить таймер 
    }
}
//-----------------------------------------------------------------------------
//Читаем из ROM номера портов для действия и идем выполнять

inline void makeAction(void) {
    uint8_t num1, num2;
    //читаем номера out портов из EEPROM для (действия)обновления 
    num2 = (uint8_t) (regCnt << 3) + bitCnt; //сквозная нумерация 0 до ALL
    num2 = ee.altPin[num2]; //читаем порт1 и порт 2
    num1 = num2 & 0x0F; //номер первого действия на порту 0-15
    num2 = (uint8_t) (num2 >> 4); //номер второго действия на порту 0-15
    doAct(num1);
    if (!(num1 == num2)) doAct(num2);
}
//-----------------------------------------------------------------------------
//вышли из debounce. Смотрим по маске ee.flagAction для каких изменившихся бит
//требуется действие в программе.

//Имеем 
//cntReg - текущий номер регистра ввода, 
//val, actBits - изменненые биты
// biCnt - глобальный номер бита
//bitMask - глобальная маска бита

void lookActInpBit(uint8_t actBits) {
    uint8_t tmp;
    bitMask = 0x80;
    bitCnt = 8;
    tmp = ee.flagAction[regCnt]; //и смотрим есть ли действие на out пины
    while (bitCnt) {
        bitCnt--; //перебираем биты (пины)
        if (actBits & bitMask) { //смотрим какие активны
            if (tmp & bitMask) makeAction(); //есть !! Идем действоать.
        }
        bitMask = (uint8_t) (bitMask >> 1);
    }

}
