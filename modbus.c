#include <xc.h>
#include "modbus.h"
#include "const.h"
//#include "timers.h"
//------------------------------------------------------------------------------
/**
 * \file
 * \brief Реализует цикл работы конечного автомата протокола приема и отсылки пакетов
 * 
 * Конечный автомат имеет три сосотаяния  0-переход в режим приема;
 * 1- режим простоя (фаза приема или передачи);2- переход в режим передачи
 * Цикл переходов последовательных переходо содержит следующие шаги
 * 1- void toFinishSendHandler(void)
 * Завершение передачи (ожидание освобождения буфера передачи USART)
 * Инициация режима приема очередного пакета МОДБАС
 * 2- void toIdleStepHandler(void) - Прием пакета МОДБАС
 * Ожиданиа завершения приема/передачи очередного пакета МОДБАС
 * 3- void toSlaveStepHandler(void)
 * Обработка принятого пакета МПДБАС и формирование ответа или пакета ошибки
 * Инициация режима передачи пакета ответа МОДБАС
 * 4- void toIdleStepHandler(void) - Передача пакета МОДБАС
 * Ожиданиа завершения приема/передачи очередного пакета МОДБАС
 * далее пункт 1
 */
//------------------------------------------------------------------------------
extern __eeprom ee_t ee;
extern uint8_t busINP; //modbus регистров in
extern uint8_t busOUT; //modbus регистров Out

void initUART(uint8_t speed);

void initModbus(void) {
    uint8_t num;

    MODBUS.addrDev = ee._aDev;
    num = ee._nSpeed;
    MODBUS.gapTime = ee._tabGap[num];
    MODBUS.gapCnt = MODBUS.gapTime;
    MODBUS.stpFunc = 0;
    busINP = ee._rInp;
    busOUT = ee._bOut;
    initUART(ee._tabSpeed[num]);
}

void engineModbus(void) {
    switch (MODBUS.stpFunc) {
        case(0):
        {
            toFinishSendHandler();
            break;
        }
        case(1):
        {
            toIdleStepHandler();
            break;
        }
        case(2):
        {
            toSlaveStepHandler();
            break;
        }
    }
}
//------------------------------------------------------------------------------
// Ожиданиа завершения приема/передачи очередного пакета МОДБАС
//------------------------------------------------------------------------------

inline void toIdleStepHandler(void) {
    NOP();
}
//------------------------------------------------------------------------------
// Функция обработки модбас фрейма
//------------------------------------------------------------------------------

inline void toSlaveStepHandler(void) {
    unsigned int crc;
    //   unsigned char tmp;
    /*если обращаются к нам или широковещательныйм адресом, и в буфере больше 5 байт*/
    if ((MODBUS.buffer[0] == MODBUS.addrDev) || (MODBUS.buffer[0] == 0)) {
        /*проверяем контрольную сумму*/
        crc = countFrameCRC16(MODBUS.buffer, MODBUS.cntByte - 2);
        /*если контрольная сумма совпала, обрабатываем сообщение*/
        if ((MODBUS.buffer[MODBUS.cntByte - 2] == (crc & 0x00FF)) && (MODBUS.buffer[MODBUS.cntByte - 1] == (crc >> 8))) {
            //обрабатываем прием и формируем ответ
            switch (MODBUS.buffer[1]) {
                case 1: //Чтение дискретного вывода
                    readCoilStatus01();
                    break;
                case 2: //Чтение дискретного ввода
                    readInputStatus02();
                    break;
                case 5: //Запись дискретного вывода
                    forceSingleCoil05();
                    break;
                case 15: //Запись нескольких дискретных выводов
                    forceMultipleCoils15();
                    break;
                default: //Пришла команда не поддерживаемая, отвечаем ошибкой
                    makeErrorFrame(0x01);
                    break;
            }
            //     добавляем контрольную сумму
            crc = countFrameCRC16(MODBUS.buffer, MODBUS.sendLen - 2);
            MODBUS.buffer[MODBUS.sendLen - 2] = (unsigned char) crc;
            MODBUS.buffer[MODBUS.sendLen - 1] = (unsigned char) (crc >> 8);
            // ПОШЛИ НА ПЕРЕДАЧУ!
            PORT_TxE = SND; //шину МОДБАС переводим на передачу
            //Инициация режима передачи пакета ответа МОДБАС
            MODBUS.cntByte = 0;
            MODBUS.stpFunc = 1;
            RCSTAbits.CREN = 0; //разрешение приема (надо ли дергать??) похоже да!
            TXSTAbits.TXEN = 1; //разрешение передачи (надо ли дергать??)
            PIE1bits.TXIE = 1; //разрешаем прерывание передачи
            return;
        }
    }
    // ОСТАЕМСЯ НА ПРИЕМЕ !!!
    MODBUS.gapCnt = MODBUS.gapTime; //инициализируем интервал молчания    
    MODBUS.cntByte = 0; //указатель установить на начало пакета
    MODBUS.stpFunc = 1;
    //   RCSTAbits.CREN = 1; //разрешение приема (надо ли дергать??) НЕТ!!!
    PIE1bits.RCIE = 1; //разрешаем прерывание приема

}
//------------------------------------------------------------------------------
// Завершение передачи (ожидание освобождения буфера передачи USART)
//------------------------------------------------------------------------------

inline void toFinishSendHandler(void) {
    //ждем пока последний байт выйдет из сдвигового регистра передатчика USART
    while (!TXSTAbits.TRMT) NOP();

    // ПОШЛИ НА ПРИЕМ !  
    TXSTAbits.TXEN = 0; //запрещение передачи (надо ли дергать??)
    PORT_TxE = RCV; //шину МОДБАС переводим на прием
    // Инициация режима приема очередного пакета МОДБАС
    MODBUS.gapCnt = MODBUS.gapTime; //инициализируем интервал молчания    
    MODBUS.cntByte = 0; //указатель установить на начало пакета
    MODBUS.stpFunc = 1;
    RCSTAbits.CREN = 1; //разрешение приема (надо ли дергать??) здесь да!
    PIE1bits.RCIE = 1; //разрешаем прерывание приема
}
//------------------------------------------------------------------------------
// Расчета контрольной суммы
//------------------------------------------------------------------------------

unsigned int countFrameCRC16(volatile unsigned char *ptrByte, int byte_cnt) {
    unsigned int w = 0;
    char shift_cnt;
    if (ptrByte) {
        w = 0xffffU;
        for (; byte_cnt > 0; byte_cnt--) {
            w = (unsigned int) ((w / 256U)*256U + ((w % 256U)^(*ptrByte++)));
            for (shift_cnt = 0; shift_cnt < 8; shift_cnt++) {
                if ((w & 0x1) == 1)
                    w = (unsigned int) ((w >> 1)^0xa001U);
                else
                    w >>= 1;
            }
        }
    }
    return w;
}

//------------------------------------------------------------------------------
//  Фукция формирования ошибки
//------------------------------------------------------------------------------

void makeErrorFrame(unsigned char error_type) {
    MODBUS.buffer[1] |= 128; //Отправляем ошибку
    MODBUS.buffer[2] = error_type; //код ошибки
    MODBUS.sendLen = ERR_PACK_SIZE; //длина пакета ошибки
}



