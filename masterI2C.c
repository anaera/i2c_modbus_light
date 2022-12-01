/*
 * File:   my_master.c
 * Author: 1
 *
 * Created on March 19, 2022, 4:00 PM
 */
#include <xc.h>
#include "const.h"
#include "masterI2C.h"

void exitI2C(void);
/*
extern volatile uint8_t regAct; //маска активных регистров
extern uint8_t regMask; //маска текущего регистра
extern uint8_t regCnt; //текущий регистр
extern COMMON_DATA COMMON;
*/
extern core_i2c_t wrkReg;
volatile i2c_states_t i2cState;

/**
 * \brief  Краткое описание
 *
 * \details  разные детали
 * \note заметки
 * \param[in]  a описание
 * \param[in.out]   b описание
 * \return описание
 * \retval возвращаемые значения
 */

/**
 * \brief Занять шину и инициировать MSSP
 */
//void I2C_Open(void) {
//    I2C_MasterOpen();
//    i2c_state = I2C_RESET;
//    I2C_MasterEnableIrq();
//}

/**
 * \brief Освободить шину и выключить MSSP
 */
void I2C_Close(void) {
    I2C_MasterDisableIrq();
    I2C_MasterClose();

}

/**
 * \brief Чтение шины I2C
 */
void I2C_Read(void) {
    I2C_MasterRead();
}

/**
 * \brief Запись шины I2C
 */
void I2C_Write(void) {
    I2C_MasterWrite();
}

/* I2C Register Level interfaces */


/**
 * \brief  Занять MSSP
 */
//static inline void I2C_MasterOpen(void) {
//    SSP1CON1bits.SSPEN = 1;
//}

/**
 *  \brief Освободить MSSP
 */

static inline void I2C_MasterClose(void) {
    //Disable I2C
    SSP1CON1bits.SSPEN = 0;
}

/**
 * \brief Инициировать прием
 */
static inline void I2C_MasterRead(void) {
    i2cState = I2C_ADDR_RECV;
    I2C_MasterStart();
    return;
}

/**
 * \brief Инициировать передачу
 */
static inline void I2C_MasterWrite(void) {
    i2cState = I2C_ADDR_SEND;
    I2C_MasterStart();
    return;
}

/**
 * \brief Разрешить работу MSSP
 */
static inline void I2C_MasterStart(void) {
    SSP1CON2bits.SEN = 1;
}

/**
 * \brief Загрузить данные в регистр передачи
 * \param[in] data данные в SSP1BUF
 */
static inline void I2C_MasterSendTxData(uint8_t data) {
    SSP1BUF = data;
}

/**
 * \brief Начать прием данных
 */
static inline void I2C_MasterStartRx(void) {
    SSP1CON2bits.RCEN = 1;
}

/**
 * \brief Выгрузить принятые данные
 */
static inline uint8_t I2C_MasterGetRxData(void) {
    return SSP1BUF;
}

/**
 * \brief Прочитать бит NACK
 */
static inline bool I2C_MasterIsNack(void) {
    return SSP1CON2bits.ACKSTAT;
}

/**
 * \brief Выставить на шину ACK
 */
//static inline void I2C_MasterSendAck(void) {
//    SSP1CON2bits.ACKDT = 0;
//    SSP1CON2bits.ACKEN = 1;
//}

/**
 * \brief Выставить на шину NACK
 */
static inline void I2C_MasterSendNack(void) {
    SSP1CON2bits.ACKDT = 1;
    SSP1CON2bits.ACKEN = 1;
}

/**
 * \brief Завершить прием / передачу STOP
 */
static inline void I2C_MasterStop(void) {
    SSP1CON2bits.PEN = 1;
}

/**
 * \brief Разрешить прерывания MSSP
 */
//static inline void I2C_MasterEnableIrq(void) {
//    PIR1bits.SSP1IF = 0;
//    PIE1bits.SSP1IE = 1;
//}

/**
 * \brief Запретить прерывания MSSP
 */
static inline void I2C_MasterDisableIrq(void) {
    PIE1bits.SSP1IE = 0;
}

/**
 * \brief Очистить флаг готовности
 */
static inline void I2C_MasterClearIrq(void) {
    PIR1bits.SSP1IF = 0;
}


//-------------------------------------------------------------------
//

void I2C_DO_ADDR_SEND(void) {
    //   i2c_status.isASK = 1;
    I2C_MasterSendTxData((uint8_t) (wrkReg.addr << 1));
    i2cState = I2C_TX_DATA;
}

void I2C_DO_ADDR_RECV(void) {
    //    i2cStatus.isASK = 1;
    I2C_MasterSendTxData((uint8_t) (wrkReg.addr << 1 | 1));
    i2cState = I2C_RCEN;
}

void I2C_DO_SEND_DATA(void) {
    if (I2C_MasterIsNack()) { // контроль ACK
        I2C_DO_SEND_STOP(); // если нетподтвеждения шлем STOP
        return;
    }
    I2C_MasterSendTxData(wrkReg.data);
    i2cState = I2C_SEND_STOP; //независимо от ASK после отсылки шлем стоп
}

/*
void I2C_DO_CHECK_NACK(void) {
    //    i2c_error = I2C_FAIL;
    I2C_DO_SEND_STOP();
}
 */
void I2C_DO_RCEN(void) {
    if (I2C_MasterIsNack()) { // контроль ACK
        I2C_DO_SEND_STOP(); // если нетподтвеждения шлем STOP
        return;
    }
    I2C_MasterStartRx();
    i2cState = I2C_RX_DATA;
}

//void I2C_DO_RX_ACK(void) {
//    I2C_MasterSendAck();
//    i2cState = I2C_RCEN;
//}

void I2C_DO_RX_NACK_STOP(void) {
    I2C_MasterSendNack();
    i2cState = I2C_SEND_STOP;
}

void I2C_DO_RX(void) {
    wrkReg.data = I2C_MasterGetRxData();
    //    debounce(I2C_MasterGetRxData());
    I2C_DO_RX_NACK_STOP();
}

void I2C_DO_SEND_STOP(void) {
    I2C_MasterStop();
    i2cState = I2C_IDLE;
}

void I2C_DO_IDLE(void) {
    //   i2c_status.busy = false;
    i2cState = I2C_RESET;
    exitI2C();
}

void I2C_DO_RESET(void) {
    //    i2c_status.busy = false;
    //   i2c_error = I2C_NOERR;
    i2cState = I2C_RESET;

}

// Автомат состояний MSSP

void I2C_MasterIsr(void) {

    I2C_MasterClearIrq();

    switch (i2cState) {

        case I2C_ADDR_SEND: //отсылаем адрес ждем OK и контроль ACK ->I2C_TX_DATA
        {
            I2C_DO_ADDR_SEND();
            return;
        }
        case I2C_ADDR_RECV: //отсылаем адрес ждем OK и контроль ACK->I2C_RCEN
        {
            I2C_DO_ADDR_RECV();
            return;
        }
        case I2C_TX_DATA: // контроль ACK
        {
            I2C_DO_SEND_DATA(); //отсылаем данные ждем OK и контроль ACK ->I2C_CHECK_TX
            return;
        }
        case I2C_RCEN: // контроль ACK
        {
            I2C_DO_RCEN(); // начинаем прием данныех, ждем OK ->I2C_RX_DATA
            return;
        }
        case I2C_RX_DATA:
        {
            I2C_DO_RX(); //забираем данные, шлем NASK + STOP, ждем OK-> IDLE
            return;

        }
        case I2C_SEND_STOP:
        {
            I2C_DO_SEND_STOP(); // шлем STOP,ждем OK -> IDLE
            return;
        }

        case I2C_IDLE:
        {
            I2C_DO_IDLE();
            return;
        }

        case I2C_RESET:
        {
            I2C_DO_RESET();
            return;
        }

    }
}
