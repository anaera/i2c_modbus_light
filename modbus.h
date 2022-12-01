#ifndef MODBUS_H
#define	MODBUS_H
#include "const.h"
/*Определим UART структура Modbus*/

/*!
 *  \brief Структура данных управления автоматом МОДБАС
 */
typedef struct {
    volatile unsigned char addrDev; /*!< адрес устройства */
    volatile unsigned char stpFunc; /*!< состояние автомата */
    
    volatile unsigned char gapTime; /*!< величина защитного интервала */
    volatile unsigned char gapCnt; /*!< счетчик защитного интервала */

    volatile unsigned char sendLen; /*!< размер отсылаемого пакета */
    volatile unsigned char cntByte; /*!< счетчик байт приема - передачи */
    volatile unsigned char buffer[MAX_PACK_SIZE + 1]; /*!< буфер приема/отправки пакета */
} MODBUS_DATA;

MODBUS_DATA MODBUS;

/*Объявление прототипов функции*/
void initModbus(void);
void readCoilStatus01(void);
void readInputStatus02(void);
void readDigitalReg(char, char, char);
//void readHoldingRegisters03(void);
//void readInputRegisters04(void);
//void readAnalogReg(char, char, char);
void forceSingleCoil05(void);
//void presetSingleRegister06(void);
void forceMultipleCoils15(void);
//void presetMultipleRegisters16(void);
void makeErrorFrame(unsigned char error_type);
unsigned int countFrameCRC16(volatile unsigned char *ptrByte, int byte_cnt);

void engineModbus(void);
inline void toIdleStepHandler(void);
inline void toSlaveStepHandler(void);
inline void toFinishSendHandler(void);

#endif

