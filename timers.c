/*
 * File:   timers.c
 * Author: 1
 *
 * Created on 23 июля 2022 г., 19:36
 */

#include"const.h"
#include "timers.h"

extern void (*loopFunc)(void);
extern void checkLoop(void);

void startTimers(void) {
    TIMERS.stpTime = STEP_TIME;
    TIMERS.secTime = WAIT_TIME;
    T1CONbits.TMR1ON = 1; // TMR1 разрешен 
}

inline void timeCount(void) {
    if (TIMERS.stpTime > 0) {
        TIMERS.stpTime--;


    } else {
        TIMERS.stpTime = STEP_TIME;
        TIMERS.flagTimers.STPFLG = 1;
 //       PORT_TxE = ~PORT_TxE;
        loopFunc = checkLoop;
        if (TIMERS.secTime > 0) {
            TIMERS.secTime--;
        } else {
            TIMERS.secTime = WAIT_TIME;
            TIMERS.flagTimers.SECFLG = 1;


        }
    }
};
