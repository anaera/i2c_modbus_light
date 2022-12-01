#ifndef I2C_MASTER_H
#define I2C_MASTER_H

#include <xc.h> // include processor files - each processor file is guarded.
//#include <stdio.h>
//#include <stdint.h>
#include <stdbool.h>


//рабочая область драйвера I2C и PIC регистров
/*
typedef union {
    uint8_t status;

    struct {
        unsigned read : 1;
        unsigned restart : 1;
        unsigned isASK : 1;
        unsigned : 1;
        unsigned : 1;
        unsigned : 1;
        unsigned inUse : 1;
        unsigned busy : 1;

    };
} status_t;
typedef enum {
    I2C_WRITE_OK, // The data was sent.
    I2C_READ_OK, // The data was received.
    I2C_ADDR_ER, // The address was not sent.
    I2C_DATA_ER, // The data was not sent.
    I2C_NOERR, // The message was sent.
    I2C_BUSY, // Message was not sent, bus was busy.
    I2C_FAIL // Message was not sent, bus failure
    // If you are interested in the failure reason,
    // Sit on the event call-backs.
} i2c_error_t;

//-------------------------------------------------------------------
*/
typedef enum {
    I2C_IDLE = 0,
    //    I2C_CHECK_NACK,
    I2C_ADDR_SEND,
    I2C_ADDR_RECV,
    I2C_TX_DATA,
    I2C_RX_DATA,
    I2C_RCEN,
    I2C_SEND_STOP,
    I2C_RESET
} i2c_states_t;

//-------------------------------------------------------------------
// Инициализация регистроов MSSP
void I2C_Init(void);
//Занять шину и инициировать MSSP
void I2C_Open(void);
//Освободить шину и выключить MSSP
void I2C_Close(void);
// Читать шину
void I2C_Read(void);
// Писать шину
void I2C_Write(void);
// Инициировать и занять MSSP
static inline void I2C_MasterOpen(void);
// Освободить MSSP
static inline void I2C_MasterClose(void);
// Инициировать прием
static inline void I2C_MasterRead(void);
// Инициировать передачу
static inline void I2C_MasterWrite(void);
// Разрешить работу MSSP
static inline void I2C_MasterStart(void);
// Загрузить данные в регистр передачи
static inline void I2C_MasterSendTxData(uint8_t data);
// Начать прием данных
static inline void I2C_MasterStartRx(void);
// Выгрузить принятые данные
static inline uint8_t I2C_MasterGetRxData(void);
// Прочитать бит NACK
static inline bool I2C_MasterIsNack(void);
// Выставить на шину ACK
static inline void I2C_MasterSendAck(void);
// Выставить на шину NACK
static inline void I2C_MasterSendNack(void);
// Завершить прием / передачу STOP
static inline void I2C_MasterStop(void);
// Разрешить прерывания MSSP
static inline void I2C_MasterEnableIrq(void);
// Запретить прерывания MSSP
static inline void I2C_MasterDisableIrq(void);
// Очистить флаг готовности 
static inline void I2C_MasterClearIrq(void);
// Автомат I2C
void I2C_MasterIsr(void);
void I2C_MasterWaitForEvent(void);
//-------------------------------------------------------------------
// Автомат состояний MSSP
void I2C_DO_ADDR_SEND(void);
void I2C_DO_ADDR_RECV(void);
void I2C_DO_SEND_DATA(void);
void I2C_DO_RCEN(void);
void I2C_DO_RX_ACK(void);
void I2C_DO_RX_NACK_STOP(void);
void I2C_DO_RX(void);
void I2C_DO_SEND_STOP(void);
void I2C_DO_IDLE(void);
void I2C_DO_RESET(void);
void I2C_MasterIsr(void);
#endif //I2C_MASTER_H